// SPDX-License-Identifier: Apache-2.0

// xstudio headers first: the Apple headers further down define a 'nil' macro that
// breaks caf headers included after them.
#include "vt_decoder.hpp"
#include "xstudio/media_reader/image_buffer.hpp"
#include "xstudio/utility/logging.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <map>
#include <mutex>
#include <set>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <VideoToolbox/VideoToolbox.h>

namespace xstudio::media_reader::ffmpeg {

namespace {

    // VideoToolbox output formats and their FFmpeg-equivalent native layouts.
    constexpr OSType ks4as = kCVPixelFormatType_444YpCbCr16VideoRange_16A_TriPlanar; // 's4as'
    enum class Packing { Bits8, Bits10InMsbs, Bits16, S4as };
    enum class Chroma { C420, C422, C444 };
    enum class VtColorSpace { Unspecified, Bt601, Bt709, Bt2020, Smpte240 };

    struct OutputFormat {
        OSType vt;
        Packing packing;
        Chroma chroma;
        bool full_range;
    };

    const OutputFormat k_output_formats[] = {
        {kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange,
         Packing::Bits8,
         Chroma::C420,
         false},
        {kCVPixelFormatType_420YpCbCr8BiPlanarFullRange,
         Packing::Bits8,
         Chroma::C420,
         true},
        {kCVPixelFormatType_422YpCbCr8BiPlanarVideoRange,
         Packing::Bits8,
         Chroma::C422,
         false},
        {kCVPixelFormatType_422YpCbCr8BiPlanarFullRange,
         Packing::Bits8,
         Chroma::C422,
         true},
        {kCVPixelFormatType_444YpCbCr8BiPlanarVideoRange,
         Packing::Bits8,
         Chroma::C444,
         false},
        {kCVPixelFormatType_444YpCbCr8BiPlanarFullRange,
         Packing::Bits8,
         Chroma::C444,
         true},
        {kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange,
         Packing::Bits10InMsbs,
         Chroma::C420,
         false},
        {kCVPixelFormatType_420YpCbCr10BiPlanarFullRange,
         Packing::Bits10InMsbs,
         Chroma::C420,
         true},
        {kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange,
         Packing::Bits10InMsbs,
         Chroma::C422,
         false},
        {kCVPixelFormatType_422YpCbCr10BiPlanarFullRange,
         Packing::Bits10InMsbs,
         Chroma::C422,
         true},
        {kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange,
         Packing::Bits10InMsbs,
         Chroma::C444,
         false},
        {kCVPixelFormatType_444YpCbCr10BiPlanarFullRange,
         Packing::Bits10InMsbs,
         Chroma::C444,
         true},
        {kCVPixelFormatType_422YpCbCr16BiPlanarVideoRange,
         Packing::Bits16,
         Chroma::C422,
         false},
        {kCVPixelFormatType_444YpCbCr16BiPlanarVideoRange,
         Packing::Bits16,
         Chroma::C444,
         false},
        // the target here is for 4444 with alpha; without, it's YUV444P12LE
        {ks4as, Packing::S4as, Chroma::C444, false},
    };

    const OutputFormat *output_format(const OSType vt) {
        for (const auto &f : k_output_formats)
            if (f.vt == vt)
                return &f;
        return nullptr;
    }

    std::string fourcc_string(const uint32_t f) {
        std::string s;
        for (int shift = 24; shift >= 0; shift -= 8) {
            const char c = char((f >> shift) & 0xff);
            s += (c >= 32 && c < 127) ? c : '?';
        }
        return s;
    }

    // ProRes codec_tag ('apch' etc., stored little-endian by FFmpeg) to the
    // big-endian CMVideoCodecType VideoToolbox wants. 0 if it isn't a ProRes tag
    // VideoToolbox decodes.
    CMVideoCodecType prores_codec_type(const uint32_t tag) {
        const uint32_t t =
            ((tag & 0xff) << 24) | ((tag & 0xff00) << 8) | ((tag >> 8) & 0xff00) | (tag >> 24);
        switch (t) {
        case kCMVideoCodecType_AppleProRes422Proxy:
        case kCMVideoCodecType_AppleProRes422LT:
        case kCMVideoCodecType_AppleProRes422:
        case kCMVideoCodecType_AppleProRes422HQ:
        case kCMVideoCodecType_AppleProRes4444:
        case kCMVideoCodecType_AppleProRes4444XQ:
            return t;
        default:
            return 0;
        }
    }

    bool is_prores_4444(const CMVideoCodecType t) {
        return t == kCMVideoCodecType_AppleProRes4444 ||
               t == kCMVideoCodecType_AppleProRes4444XQ;
    }

    struct Choice {
        CMVideoCodecType codec_type = 0;
        bool prores_4444            = false;
    };

    // Decides whether VideoToolbox should decode the stream. VideoToolbox selects
    // the output pixel format from the codec and decoded frame.
    bool choose(const AVStream *stream, Choice &choice, std::string &reason) {

        const AVCodecParameters *par = stream->codecpar;
        if (par->codec_type != AVMEDIA_TYPE_VIDEO) {
            reason = "not a video stream";
            return false;
        }
        if (par->width <= 0 || par->height <= 0) {
            reason = "unknown picture size";
            return false;
        }
        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            reason = "attached picture";
            return false;
        }

        if (par->codec_id == AV_CODEC_ID_PRORES) {
            choice.codec_type = prores_codec_type(par->codec_tag);
            if (!choice.codec_type) {
                reason = "unrecognised ProRes codec tag";
                return false;
            }
            choice.prores_4444 = is_prores_4444(choice.codec_type);
            return true;
        }

        if (par->codec_id != AV_CODEC_ID_H264 && par->codec_id != AV_CODEC_ID_HEVC) {
            reason = std::string("codec ") + avcodec_get_name(par->codec_id) +
                     " is not decoded with VideoToolbox";
            return false;
        }

        // A length-prefixed stream carries an avcC/hvcC record (first byte 1). An
        // Annex-B stream (raw .h264/.ts) has start codes instead, or nothing.
        if (par->extradata_size < 7 || par->extradata[0] != 1) {
            reason = "no avcC/hvcC decoder configuration (Annex-B stream)";
            return false;
        }

        choice.codec_type =
            par->codec_id == AV_CODEC_ID_H264 ? kCMVideoCodecType_H264 : kCMVideoCodecType_HEVC;
        return true;
    }

    // Does the first ProRes frame carry alpha? From the frame header, the same
    // way FFmpeg's decoder decides between yuv444p12 and yuva444p12.
    bool prores_frame_has_alpha(const AVPacket *pkt) {
        // 4 byte frame size, 'icpf', then the frame header; alpha_info is the low
        // nibble of header byte 17.
        if (!pkt || pkt->size < 8 + 18)
            return false;
        if (memcmp(pkt->data + 4, "icpf", 4))
            return false;
        return (pkt->data[8 + 17] & 0x0f) != 0;
    }

    void populate_vt_shader_params(
        utility::JsonStore &params,
        const OutputFormat &format,
        const VtColorSpace colorspace,
        const int sample_storage_bits,
        const bool interleaved_chroma) {
        static const Imath::M33f bt601(
            1., 0., 1.402, 1., -0.34413629, -0.71413629, 1., 1.772, -0.);
        static const Imath::M33f bt2020(
            1., -0., 1.4746, 1., -0.16455313, -0.57135313, 1., 1.8814, 0.);
        static const Imath::M33f bt709(
            1., 0., 1.5748, 1., -0.18732427, -0.46812427, 1., 1.8556, 0.);

        const int bits = sample_storage_bits;
        const int max_cv = (1 << bits) - 1;
        params["half_scale_uvx"]    = format.chroma != Chroma::C444;
        params["half_scale_uvy"]    = format.chroma == Chroma::C420;
        params["norm_coeff"]        = 1.0f / float(max_cv);
        params["vt_chroma_interleaved"] = interleaved_chroma;

        Imath::M33f yuv_to_rgb;
        switch (colorspace) {
        case VtColorSpace::Bt2020:
            yuv_to_rgb = bt2020;
            break;
        case VtColorSpace::Bt709:
            yuv_to_rgb = bt709;
            break;
        case VtColorSpace::Smpte240:
        case VtColorSpace::Bt601:
        case VtColorSpace::Unspecified:
        default:
            yuv_to_rgb = bt601;
            break;
        }

        if (format.full_range) {
            const float neutral = std::pow(2.0f, float(bits - 1));
            params["yuv_offsets"] = {"ivec3", 1, 0.0f, neutral, neutral};
        } else {
            const float y_min  = 16.0f * std::pow(2.0f, float(bits - 8));
            const float y_max  = 235.0f * std::pow(2.0f, float(bits - 8));
            const float uv_min = 16.0f * std::pow(2.0f, float(bits - 8));
            const float uv_max = 240.0f * std::pow(2.0f, float(bits - 8));
            Imath::M33f scale;
            scale[0][0] = float(max_cv) / (y_max - y_min);
            scale[1][1] = float(max_cv) / (uv_max - uv_min);
            scale[2][2] = float(max_cv) / (uv_max - uv_min);
            yuv_to_rgb *= scale;
            params["yuv_offsets"] = {
                "ivec3",
                1,
                y_min,
                128.0f * std::pow(2.0f, float(bits - 8)),
                128.0f * std::pow(2.0f, float(bits - 8))};
        }
        params["yuv_conv"] = yuv_to_rgb.transposed();
    }

    bool cf_equal(CFTypeRef value, CFStringRef key) {
        return value && CFGetTypeID(value) == CFStringGetTypeID() &&
               CFStringCompare((CFStringRef)value, key, 0) == kCFCompareEqualTo;
    }

    // Colour metadata VideoToolbox attached to a decoded buffer, for fields the
    // container left unspecified (design D6).
    void colour_from_attachments(
        CVPixelBufferRef pb,
        VtColorSpace &colorspace,
        AVColorPrimaries &color_primaries,
        AVColorTransferCharacteristic &color_trc) {
        if (colorspace == VtColorSpace::Unspecified) {
            CFTypeRef m = CVBufferCopyAttachment(pb, kCVImageBufferYCbCrMatrixKey, nullptr);
            if (cf_equal(m, kCVImageBufferYCbCrMatrix_ITU_R_709_2))
                colorspace = VtColorSpace::Bt709;
            else if (cf_equal(m, kCVImageBufferYCbCrMatrix_ITU_R_601_4))
                colorspace = VtColorSpace::Bt601;
            else if (cf_equal(m, kCVImageBufferYCbCrMatrix_ITU_R_2020))
                colorspace = VtColorSpace::Bt2020;
            else if (cf_equal(m, kCVImageBufferYCbCrMatrix_SMPTE_240M_1995))
                colorspace = VtColorSpace::Smpte240;
            if (m)
                CFRelease(m);
        }

        if (color_primaries == AVCOL_PRI_UNSPECIFIED) {
            CFTypeRef p = CVBufferCopyAttachment(pb, kCVImageBufferColorPrimariesKey, nullptr);
            if (cf_equal(p, kCVImageBufferColorPrimaries_ITU_R_709_2))
                color_primaries = AVCOL_PRI_BT709;
            else if (cf_equal(p, kCVImageBufferColorPrimaries_ITU_R_2020))
                color_primaries = AVCOL_PRI_BT2020;
            else if (cf_equal(p, kCVImageBufferColorPrimaries_P3_D65))
                color_primaries = AVCOL_PRI_SMPTE432;
            else if (cf_equal(p, kCVImageBufferColorPrimaries_DCI_P3))
                color_primaries = AVCOL_PRI_SMPTE431;
            else if (cf_equal(p, kCVImageBufferColorPrimaries_EBU_3213))
                color_primaries = AVCOL_PRI_BT470BG;
            else if (cf_equal(p, kCVImageBufferColorPrimaries_SMPTE_C))
                color_primaries = AVCOL_PRI_SMPTE170M;
            if (p)
                CFRelease(p);
        }
        if (color_trc == AVCOL_TRC_UNSPECIFIED) {
            CFTypeRef t =
                CVBufferCopyAttachment(pb, kCVImageBufferTransferFunctionKey, nullptr);
            if (cf_equal(t, kCVImageBufferTransferFunction_ITU_R_709_2))
                color_trc = AVCOL_TRC_BT709;
            else if (cf_equal(t, kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ))
                color_trc = AVCOL_TRC_SMPTE2084;
            else if (cf_equal(t, kCVImageBufferTransferFunction_ITU_R_2100_HLG))
                color_trc = AVCOL_TRC_ARIB_STD_B67;
            else if (cf_equal(t, kCVImageBufferTransferFunction_ITU_R_2020))
                color_trc = AVCOL_TRC_BT2020_10;
            else if (cf_equal(t, kCVImageBufferTransferFunction_SMPTE_240M_1995))
                color_trc = AVCOL_TRC_SMPTE240M;
            if (t)
                CFRelease(t);
        }
    }

    VtColorSpace vt_colorspace_from_stream(const AVColorSpace colorspace) {
        switch (colorspace) {
        case AVCOL_SPC_BT709:
            return VtColorSpace::Bt709;
        case AVCOL_SPC_BT2020_NCL:
        case AVCOL_SPC_BT2020_CL:
            return VtColorSpace::Bt2020;
        case AVCOL_SPC_SMPTE240M:
            return VtColorSpace::Smpte240;
        case AVCOL_SPC_BT470BG:
        case AVCOL_SPC_SMPTE170M:
            return VtColorSpace::Bt601;
        default:
            return VtColorSpace::Unspecified;
        }
    }

    const char *vt_colorspace_name(const VtColorSpace colorspace) {
        switch (colorspace) {
        case VtColorSpace::Bt709:
            return "bt709";
        case VtColorSpace::Bt2020:
            return "bt2020";
        case VtColorSpace::Smpte240:
            return "smpte240";
        case VtColorSpace::Bt601:
        case VtColorSpace::Unspecified:
            return "bt601";
        }
    }

} // namespace

struct VtDecoder::Impl {
    VTDecompressionSessionRef session  = nullptr;
    CMVideoFormatDescriptionRef format = nullptr;
    Choice choice;
    AVRational time_base = {1, 1};
    OSType output_format = 0;
    bool has_alpha       = false;
    bool first_packet    = true;
    bool hardware        = false;
    VtColorSpace colorspace = VtColorSpace::Unspecified;
    AVColorPrimaries primaries;
    AVColorTransferCharacteristic trc;

    std::mutex mutex;
    // Presentation timestamps of packets submitted that VideoToolbox hasn't
    // called back for yet
    std::set<int64_t> pending;
    // Frames VideoToolbox has returned, by presentation timestamp
    std::map<int64_t, CVPixelBufferRef> decoded;
    // Highest decode timestamp submitted. Every later packet has pts >= its
    // dts >= this, so a decoded frame with pts <= max_dts can't be preceded by
    // anything still to come (design D8).
    int64_t max_dts = std::numeric_limits<int64_t>::min();
    bool draining   = false;

    ~Impl() {
        clear_decoded();
        if (session) {
            VTDecompressionSessionWaitForAsynchronousFrames(session);
            VTDecompressionSessionInvalidate(session);
            CFRelease(session);
        }
        if (format)
            CFRelease(format);
    }

    void clear_decoded() {
        std::lock_guard<std::mutex> l(mutex);
        for (auto &p : decoded)
            CVPixelBufferRelease(p.second);
        decoded.clear();
        pending.clear();
        max_dts = std::numeric_limits<int64_t>::min();
    }

    static void output_callback(
        void *refcon,
        void *source_frame_refcon,
        OSStatus status,
        VTDecodeInfoFlags flags,
        CVImageBufferRef image,
        CMTime,
        CMTime) {
        auto *self        = static_cast<Impl *>(refcon);
        const int64_t pts = int64_t(reinterpret_cast<intptr_t>(source_frame_refcon));
        std::lock_guard<std::mutex> l(self->mutex);
        self->pending.erase(pts);
        if (status != noErr || (flags & kVTDecodeInfo_FrameDropped) || !image) {
            // Undecodable (e.g. an open-GOP leading picture after a seek); it is
            // simply never output.
            return;
        }
        auto it = self->decoded.find(pts);
        if (it != self->decoded.end())
            CVPixelBufferRelease(it->second);
        self->decoded[pts] = CVPixelBufferRetain(image);
    }
};

VtDecoder::VtDecoder(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

VtDecoder::~VtDecoder() = default;

bool VtDecoder::eligible(const AVStream *stream, std::string &reason) {
    Choice choice;
    return choose(stream, choice, reason);
}

std::unique_ptr<VtDecoder> VtDecoder::create(const AVStream *stream, std::string &error) {

    auto impl = std::make_unique<Impl>();
    if (!choose(stream, impl->choice, error))
        return nullptr;

    // Debugging aid, and how the fallback/failure paths are tested: pretend
    // VideoToolbox can't create a session.
    if (getenv("XSTUDIO_VIDEOTOOLBOX_FORCE_FAILURE")) {
        error = "session creation disabled by XSTUDIO_VIDEOTOOLBOX_FORCE_FAILURE";
        return nullptr;
    }

    const AVCodecParameters *par = stream->codecpar;
    impl->time_base              = stream->time_base;
    impl->colorspace             = vt_colorspace_from_stream(par->color_space);
    impl->primaries              = par->color_primaries;
    impl->trc                    = par->color_trc;
    // Until the first ProRes frame header says otherwise, 4444 is assumed to have alpha.
    impl->has_alpha = impl->choice.prores_4444;

    CFMutableDictionaryRef extensions = CFDictionaryCreateMutable(
        nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (par->codec_id == AV_CODEC_ID_H264 || par->codec_id == AV_CODEC_ID_HEVC) {
        CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(
            nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFDataRef data = CFDataCreate(nullptr, par->extradata, par->extradata_size);
        CFDictionarySetValue(
            atoms, par->codec_id == AV_CODEC_ID_H264 ? CFSTR("avcC") : CFSTR("hvcC"), data);
        CFDictionarySetValue(
            extensions, kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms, atoms);
        CFRelease(data);
        CFRelease(atoms);
    }
    OSStatus status = CMVideoFormatDescriptionCreate(
        nullptr, impl->choice.codec_type, par->width, par->height, extensions, &impl->format);
    CFRelease(extensions);
    if (status != noErr) {
        error = fmt::format("CMVideoFormatDescriptionCreate failed ({})", int(status));
        return nullptr;
    }

    CFMutableDictionaryRef spec = CFDictionaryCreateMutable(
        nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    // Use hardware when there is some; VideoToolbox falls back to its own software
    // decoder (e.g. ProRes on Intel) otherwise.
    CFDictionarySetValue(
        spec,
        kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder,
        kCFBooleanTrue);

    CFMutableArrayRef supported_formats = CFArrayCreateMutable(
        nullptr, 0, &kCFTypeArrayCallBacks);
    for (const auto &format : k_output_formats) {
        const SInt32 value = SInt32(format.vt);
        CFNumberRef number = CFNumberCreate(nullptr, kCFNumberSInt32Type, &value);
        CFArrayAppendValue(supported_formats, number);
        CFRelease(number);
    }
    CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(
        nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (impl->choice.prores_4444) {
        const SInt32 value = SInt32(ks4as);
        CFNumberRef number = CFNumberCreate(nullptr, kCFNumberSInt32Type, &value);
        CFDictionarySetValue(attributes, kCVPixelBufferPixelFormatTypeKey, number);
        CFRelease(number);
    } else {
        CFDictionarySetValue(
            attributes, kCVPixelBufferPixelFormatTypeKey, supported_formats);
    }
    CFRelease(supported_formats);

    VTDecompressionOutputCallbackRecord callback{&Impl::output_callback, impl.get()};
    status = VTDecompressionSessionCreate(
        nullptr, impl->format, spec, attributes, &callback, &impl->session);
    CFRelease(spec);
    CFRelease(attributes);
    if (status != noErr) {
        impl->session = nullptr;
        error         = fmt::format("VTDecompressionSessionCreate failed ({})", int(status));
        return nullptr;
    }

    CFBooleanRef hw = nullptr;
    if (VTSessionCopyProperty(
            impl->session,
            kVTDecompressionPropertyKey_UsingHardwareAcceleratedVideoDecoder,
            nullptr,
            &hw) == noErr &&
        hw) {
        impl->hardware = hw == kCFBooleanTrue;
        CFRelease(hw);
    }

    return std::unique_ptr<VtDecoder>(new VtDecoder(std::move(impl)));
}

int VtDecoder::send_packet(const AVPacket *packet) {

    Impl &d = *impl_;

    if (!packet || !packet->size) {
        // Start draining: everything submitted so far comes out of receive_frame,
        // then AVERROR_EOF.
        VTDecompressionSessionFinishDelayedFrames(d.session);
        VTDecompressionSessionWaitForAsynchronousFrames(d.session);
        std::lock_guard<std::mutex> l(d.mutex);
        d.draining = true;
        return 0;
    }

    {
        std::lock_guard<std::mutex> l(d.mutex);
        if (d.draining)
            return AVERROR_EOF;
    }

    // Reordering relies on every packet having both timestamps (D8).
    if (packet->pts == AV_NOPTS_VALUE || packet->dts == AV_NOPTS_VALUE)
        return AVERROR(ENOTSUP);

    if (d.first_packet) {
        d.first_packet = false;
        if (d.choice.prores_4444)
            d.has_alpha = prores_frame_has_alpha(packet);
    }

    // Wrap the packet data without copying: decoding is synchronous (D9), so it is
    // finished with before this returns.
    CMBlockBufferRef block = nullptr;
    OSStatus status        = CMBlockBufferCreateWithMemoryBlock(
        nullptr,
        packet->data,
        size_t(packet->size),
        kCFAllocatorNull,
        nullptr,
        0,
        size_t(packet->size),
        0,
        &block);
    if (status != noErr)
        return AVERROR(ENOMEM);

    const AVRational tb = d.time_base;
    CMSampleTimingInfo timing;
    timing.duration =
        packet->duration > 0 ? CMTimeMake(packet->duration * tb.num, tb.den) : kCMTimeInvalid;
    timing.presentationTimeStamp = CMTimeMake(packet->pts * tb.num, tb.den);
    timing.decodeTimeStamp       = CMTimeMake(packet->dts * tb.num, tb.den);
    const size_t sample_size     = size_t(packet->size);

    CMSampleBufferRef sample = nullptr;
    status                   = CMSampleBufferCreateReady(
        nullptr, block, d.format, 1, 1, &timing, 1, &sample_size, &sample);
    CFRelease(block);
    if (status != noErr)
        return AVERROR(ENOMEM);

    {
        std::lock_guard<std::mutex> l(d.mutex);
        d.pending.insert(packet->pts);
        d.max_dts = std::max(d.max_dts, packet->dts);
    }

    VTDecodeInfoFlags info = 0;
    status                 = VTDecompressionSessionDecodeFrame(
        d.session,
        sample,
        0, // synchronous
        reinterpret_cast<void *>(intptr_t(packet->pts)),
        &info);
    CFRelease(sample);

    if (status != noErr) {
        // VideoToolbox may or may not have called back with the error; either way
        // this frame won't be output.
        std::lock_guard<std::mutex> l(d.mutex);
        d.pending.erase(packet->pts);
        if (status == kVTInvalidSessionErr)
            return AVERROR_EXTERNAL;
        spdlog::debug(
            "VideoToolbox could not decode packet at pts {}: {}", packet->pts, int(status));
    }
    return 0;
}

int VtDecoder::receive_frame(ImageBufPtr &image) {

    Impl &d             = *impl_;
    CVPixelBufferRef pb = nullptr;
    int64_t pts         = 0;
    {
        std::lock_guard<std::mutex> l(d.mutex);
        if (d.decoded.empty())
            return d.draining ? AVERROR_EOF : AVERROR(EAGAIN);

        // The earliest decoded frame can go out once no frame with an earlier
        // timestamp can still turn up: none is in flight, and no packet yet to be
        // sent can have one (pts <= max_dts).
        auto first = d.decoded.begin();
        if (!d.draining && (first->first > d.max_dts ||
                            (!d.pending.empty() && first->first > *d.pending.begin())))
            return AVERROR(EAGAIN);

        pts = first->first;
        pb  = first->second;
        d.decoded.erase(first);
    }

    const OSType got       = CVPixelBufferGetPixelFormatType(pb);
    const OutputFormat *of = output_format(got);
    const bool ok          = of != nullptr;
    d.output_format        = got;
    if (ok) {
        const int w              = int(CVPixelBufferGetWidth(pb));
        const int h              = int(CVPixelBufferGetHeight(pb));
        const size_t plane_count = CVPixelBufferGetPlaneCount(pb);
        std::array<size_t, 4> plane_sizes{};
        size_t total_size = 0;
        for (size_t i = 0; i < plane_count && i < plane_sizes.size(); ++i) {
            plane_sizes[i] = CVPixelBufferGetBytesPerRowOfPlane(pb, i) *
                             CVPixelBufferGetHeightOfPlane(pb, i);
            total_size += plane_sizes[i];
        }

        image.reset(new ImageBuffer());
        auto *base = reinterpret_cast<uint8_t *>(image->allocate(total_size + 4096));
        utility::JsonStore params;
        //utility::time_point start_time = utility::clock::now();
        if (CVPixelBufferLockBaseAddress(pb, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess) {
            size_t offset = 0;
            for (size_t i = 0; i < plane_count && i < 4; ++i) {
                const auto stride = CVPixelBufferGetBytesPerRowOfPlane(pb, i);
                const auto rows   = CVPixelBufferGetHeightOfPlane(pb, i);
                const auto *src =
                    static_cast<const uint8_t *>(CVPixelBufferGetBaseAddressOfPlane(pb, i));
                const size_t plane_size = stride * rows;

                // On an M1 mac mini, this memcpy is killing performance of the decoder in some cases. My 
                // suspicion is that the decoder is running on the CPU (since I see a high CPU load on 
                // VTDecoderXPCService during playback of prores 4444 XQ). The scheduler might be moving
                // the memcpy around, causing cache misses and poor performance. Or some throttling happens
                // on RAM bandwidth. no idea yet.

                // TO DO: take ownership of the plane data directly if possible to avoid memcpy.
                std::memcpy(base + offset, src, plane_size);
                if (of->packing == Packing::S4as && i == 2) {
                    params["a_linesize"]           = int(stride);
                    params["a_plane_bytes_offset"] = offset;
                } else if (i == 0) {
                    params["y_linesize"]           = int(stride);
                    params["y_plane_bytes_offset"] = offset;
                } else if (i == 1) {
                    params["u_linesize"]           = int(stride);
                    params["u_plane_bytes_offset"] = offset;
                } else if (i == 2) {
                    params["v_linesize"]           = int(stride);
                    params["v_plane_bytes_offset"] = offset;
                }
                offset += plane_size;
            }
            CVPixelBufferUnlockBaseAddress(pb, kCVPixelBufferLock_ReadOnly);
            //utility::time_point end_time = utility::clock::now();
            //std::cerr << "Plane copy time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms" << std::endl;
        } else {
            CVPixelBufferRelease(pb);
            return AVERROR_EXTERNAL;
        }

        VtColorSpace colorspace                  = d.colorspace;
        AVColorPrimaries color_primaries        = d.primaries;
        AVColorTransferCharacteristic color_trc = d.trc;
        colour_from_attachments(pb, colorspace, color_primaries, color_trc);

        const int sample_storage_bits =
            of->packing == Packing::Bits8
                ? 8
                : (of->packing == Packing::Bits10InMsbs ? 10 : 16);
        params["vt_sample_bits"] = sample_storage_bits;
        populate_vt_shader_params(params, *of, colorspace, sample_storage_bits, true);

        utility::JsonStore metadata;
        metadata["pts"]             = pts;
        metadata["pixel_format"]    = uint32_t(got);
        metadata["color_space"]     = vt_colorspace_name(colorspace);
        metadata["color_primaries"] = int(color_primaries);
        metadata["color_trc"]       = int(color_trc);
        metadata["full_range"]      = of->full_range;
        metadata["value_bits"] =
            of->packing == Packing::S4as || of->packing == Packing::Bits16
                ? 12
                : sample_storage_bits;
        metadata["has_alpha"] = params.value("a_linesize", 0) != 0;
        image->set_image_dimensions({w, h});
        image->set_shader_params(params);
        image->set_metadata(metadata);
        image->set_display_timestamp_seconds(
            double(pts) * double(d.time_base.num) / double(d.time_base.den));
    }

    CVPixelBufferRelease(pb);

    if (!ok) {
        spdlog::warn("VideoToolbox selected unsupported pixel format {}", fourcc_string(got));
        return AVERROR_EXTERNAL;
    }
    return 0;
}

void VtDecoder::flush() {
    if (impl_->session)
        VTDecompressionSessionWaitForAsynchronousFrames(impl_->session);
    impl_->clear_decoded();
    std::lock_guard<std::mutex> l(impl_->mutex);
    impl_->draining = false;
}

std::string VtDecoder::output_format_name() const {
    return impl_->output_format ? fourcc_string(impl_->output_format) : "automatic";
}

bool VtDecoder::using_hardware() const { return impl_->hardware; }

} // namespace xstudio::media_reader::ffmpeg
