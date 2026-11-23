// SPDX-License-Identifier: Apache-2.0
#include "ffmpeg_stream_videotoolbox.hpp"

#include "vt_decoder.hpp"
#include "xstudio/media/media_error.hpp"
#include "xstudio/ui/opengl/shader_program_base.hpp"
#include "xstudio/utility/logging.hpp"

#include <mutex>
#include <set>

#include <CoreVideo/CoreVideo.h>

namespace xstudio::media_reader::ffmpeg {
namespace {

bool first_time_for(const std::string &key) {
    static std::mutex mutex;
    static std::set<std::string> seen;
    std::lock_guard<std::mutex> lock(mutex);
    return seen.insert(key).second;
}

static const utility::Uuid video_toolbox_unpack_shader_uuid{
    "CEA6C113-98D5-47D4-8385-7A8B1C936D89"};

static const std::string video_toolbox_shader_yuv_source = R"(
#version 410 core
uniform ivec2 texture_dims;
uniform int y_linesize;
uniform int crcb_linesize;
uniform int a_linesize;
uniform int y_plane_bytes_offset;
uniform int crcb_plane_bytes_offset;
uniform int a_plane_bytes_offset;
uniform bool half_scale_uvy;
uniform bool half_scale_uvx;
uniform mat3 yuv_conv;
uniform ivec3 yuv_offsets;
uniform float norm_coeff;
uniform int vt_sample_bits;

int get_image_data_2bytes(int byte_address);
int get_image_data_1byte(int byte_address);

int vt_read_sample(ivec2 coord, int offset, int stride)
{
    if (vt_sample_bits == 8)
        return get_image_data_1byte(offset + coord.x + coord.y * stride);

    int samp = get_image_data_2bytes(
        offset + coord.x * 2 + coord.y * stride);
    return vt_sample_bits == 10 ? (samp >> 6) : samp;
}

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    if (image_coord.x < image_bounds_min.x ||
        image_coord.x >= image_bounds_max.x ||
        image_coord.y < image_bounds_min.y ||
        image_coord.y >= image_bounds_max.y)
        return vec4(0.0);

    ivec2 uv_coord = ivec2(
        half_scale_uvx ? image_coord.x >> 1 : image_coord.x,
        half_scale_uvy ? image_coord.y >> 1 : image_coord.y);

    int bytes_per_sample = vt_sample_bits == 8 ? 1 : 2;
    int cb;
    int cr;
    int chroma_address =
        crcb_plane_bytes_offset +
        uv_coord.x * 2 * bytes_per_sample +
        uv_coord.y * crcb_linesize;
    cb = vt_sample_bits == 8
        ? get_image_data_1byte(chroma_address)
        : get_image_data_2bytes(chroma_address);
    cr = vt_sample_bits == 8
        ? get_image_data_1byte(chroma_address + 1)
        : get_image_data_2bytes(chroma_address + 2);
    if (vt_sample_bits == 10) {
        cb >>= 6;
        cr >>= 6;
    }

    ivec3 yuv = ivec3(
        vt_read_sample(image_coord, y_plane_bytes_offset, y_linesize),
        cb,
        cr) - yuv_offsets;
    vec3 rgb = vec3(yuv) * yuv_conv * norm_coeff;

    float alpha = 1.0;
    if (a_linesize != 0) {
        alpha = float(vt_read_sample(
            image_coord, a_plane_bytes_offset, a_linesize)) * norm_coeff;
    }

    return vec4(clamp(rgb, 0.0, 1.0), clamp(alpha, 0.0, 1.0));
}
)";

static const ui::viewport::GPUShaderPtr video_toolbox_shader_yuv(
    new ui::opengl::OpenGLShader(
        video_toolbox_unpack_shader_uuid, video_toolbox_shader_yuv_source));

} // namespace

FFMpegStreamVideoToolbox::FFMpegStreamVideoToolbox(
    AVFormatContext *fmt_ctx,
    AVStream *stream,
    int index,
    int /*thread_count*/,
    std::string path,
    const VideoToolboxMode mode)
    : FFMpegStreamInterface(fmt_ctx, stream, index, std::move(path)), mode_(mode) {

    const char *codec_name = avcodec_get_name(codec_id());
    std::string reason;
    std::unique_ptr<VtDecoder> decoder;

    eligible_ = VtDecoder::eligible(stream, reason);
    if (eligible_)
        decoder = VtDecoder::create(stream, reason);

    if (!eligible_) {
        error_ = fmt::format("VideoToolbox decoding unavailable for {}: {}", codec_name, reason);
        if (first_time_for(source_path + "|ffmpeg"))
            spdlog::debug(
                "{}: {} video decoded by FFmpeg ({})", source_path, codec_name, reason);
        return;
    }

    if (!decoder) {
        error_ = fmt::format("VideoToolbox decoding unavailable for {}: {}", codec_name, reason);
        if (mode_ == VideoToolboxMode::Prefer) {
            if (first_time_for(source_path + "|fallback"))
                spdlog::warn(
                    "{}: VideoToolbox can't decode this {} video ({}); using FFmpeg's decoder",
                    source_path,
                    codec_name,
                    reason);
        } else if (first_time_for(source_path + "|failed")) {
            spdlog::warn("{}: {}", source_path, error_);
        }
        return;
    }

    vt_decoder_ = std::move(decoder);

    if (first_time_for(source_path + "|videotoolbox"))
        spdlog::debug(
            "{}: {} video decoded by VideoToolbox (output format {}, {})",
            source_path,
            codec_name,
            vt_decoder_->output_format_name(),
            vt_decoder_->using_hardware() ? "hardware" : "software");
}

FFMpegStreamVideoToolbox::~FFMpegStreamVideoToolbox() {
    if (sws_context_)
        sws_freeContext(sws_context_);
}

int64_t FFMpegStreamVideoToolbox::current_frame() {
    if (nothing_decoded_yet_)
        return -1;
    if (current_frame_ != CURRENT_FRAME_UNKNOWN)
        return current_frame_;

    current_frame_ = frame_for_timestamp(frame_timestamp_);
    return current_frame_;
}

int64_t FFMpegStreamVideoToolbox::receive_frame() {
    if (!error_.empty())
        throw videotoolbox_unavailable_error(error_);
    if (duration_frames() == 1 && !nothing_decoded_yet_)
        return AVERROR_EOF;

    int result = vt_decoder_->receive_frame(decoded_image_);
    if (!result) {
        frame_timestamp_ = decoded_image_->metadata().value("pts", AV_NOPTS_VALUE);
        if (current_frame_ != CURRENT_FRAME_UNKNOWN)
            current_frame_++;
        if (nothing_decoded_yet_)
            nothing_decoded_yet_ = false;
    }

    if (result != AVERROR(EAGAIN) && result != AVERROR_EOF)
        AVC_CHECK_THROW(result, "videotoolbox_receive_frame");
    return result;
}

void FFMpegStreamVideoToolbox::send_flush_packet() {
    if (vt_decoder_)
        vt_decoder_->send_packet(nullptr);
}

int FFMpegStreamVideoToolbox::send_packet(AVPacket *packet) {
    if (!error_.empty()) {
        av_packet_unref(packet);
        throw videotoolbox_unavailable_error(error_);
    }
    if (!vt_decoder_) {
        av_packet_unref(packet);
        throw videotoolbox_unavailable_error(error_);
    }
    const int result = vt_decoder_->send_packet(packet);
    if (result != AVERROR(ENOTSUP)) {
        av_packet_unref(packet);
        return result;
    }

    const std::string reason = "stream packets have no timestamps";
    if (mode_ == VideoToolboxMode::Prefer)
        throw videotoolbox_fallback_error(reason);

    error_ = fmt::format(
        "VideoToolbox decoding unavailable for {}: {}", avcodec_get_name(codec_id()), reason);
    av_packet_unref(packet);
    throw videotoolbox_unavailable_error(error_);
}

void FFMpegStreamVideoToolbox::flush_buffers() {
    if (vt_decoder_)
        vt_decoder_->flush();
}

ImageBufPtr FFMpegStreamVideoToolbox::get_ffmpeg_frame_as_xstudio_image() {
    if (!decoded_image_)
        throw media_corrupt_error("VideoToolbox returned a frame without an image buffer.");

    ImageBufPtr image = decoded_image_;
    image->set_image_dimensions(resolution());

    image->set_shader(video_toolbox_shader_yuv);
    image->set_display_timestamp_seconds(pts_to_seconds(frame_timestamp_));

    const auto &params    = image->shader_params();
    const int alpha_stride = params.value("a_linesize", 0);
    image->set_has_alpha(alpha_stride != 0);
    image->set_decoder_frame_number(current_frame());
    return image;
}

std::shared_ptr<thumbnail::ThumbnailBuffer>
FFMpegStreamVideoToolbox::convert_av_frame_to_thumbnail(const size_t size_hint) {
    if (!decoded_image_)
        return {};

    const auto dims = resolution();
    const int width = dims.x > dims.y ? int(size_hint) : int(size_hint * dims.x / dims.y);
    const int height = dims.y > dims.x ? int(size_hint) : int(size_hint * dims.y / dims.x);
    const auto &params   = decoded_image_->shader_params();
    const auto &metadata = decoded_image_->metadata();
    const uint32_t native_format = metadata.value("pixel_format", uint32_t(0));

    AVPixelFormat pixel_format = AV_PIX_FMT_NONE;
    if (native_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange ||
             native_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)
        pixel_format = AV_PIX_FMT_NV12;
    else if (native_format == kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange ||
             native_format == kCVPixelFormatType_420YpCbCr10BiPlanarFullRange)
        pixel_format = AV_PIX_FMT_P010LE;
    else if (native_format == kCVPixelFormatType_422YpCbCr8BiPlanarVideoRange ||
             native_format == kCVPixelFormatType_422YpCbCr8BiPlanarFullRange)
        pixel_format = AV_PIX_FMT_NV16;
    else if (native_format == kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange ||
             native_format == kCVPixelFormatType_422YpCbCr10BiPlanarFullRange)
        pixel_format = AV_PIX_FMT_P210LE;
    else if (native_format == kCVPixelFormatType_444YpCbCr8BiPlanarVideoRange ||
             native_format == kCVPixelFormatType_444YpCbCr8BiPlanarFullRange)
        pixel_format = AV_PIX_FMT_NV24;
    else if (native_format == kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange ||
             native_format == kCVPixelFormatType_444YpCbCr10BiPlanarFullRange)
        pixel_format = AV_PIX_FMT_P410LE;
    else if (native_format == kCVPixelFormatType_422YpCbCr16BiPlanarVideoRange)
        pixel_format = AV_PIX_FMT_P216LE;
    else if (native_format == kCVPixelFormatType_444YpCbCr16BiPlanarVideoRange)
        pixel_format = AV_PIX_FMT_P416LE;
    else if (native_format == kCVPixelFormatType_444YpCbCr16VideoRange_16A_TriPlanar)
        pixel_format = AV_PIX_FMT_P416LE;
    if (pixel_format == AV_PIX_FMT_NONE)
        return {};

    sws_context_ = sws_getCachedContext(
        sws_context_,
        dims.x,
        dims.y,
        pixel_format,
        width,
        height,
        AV_PIX_FMT_RGB48LE,
        0,
        nullptr,
        nullptr,
        nullptr);
    if (!sws_context_)
        return {};

    auto thumbnail =
        std::make_shared<thumbnail::ThumbnailBuffer>(width, height, thumbnail::TF_RGBF96);
    std::vector<uint16_t> rgb(size_t(width) * height * 3);
    auto *base = reinterpret_cast<uint8_t *>(decoded_image_->buffer());
    std::array<uint8_t *, 4> planes{};
    std::array<int, 4> strides{};
    const char *offset_names[] = {
        "y_plane_bytes_offset",
        "crcb_plane_bytes_offset",
        "v_plane_bytes_offset",
        "a_plane_bytes_offset"};
    const char *stride_names[] = {"y_linesize", "crcb_linesize", "v_linesize", "a_linesize"};
    for (int i = 0; i < 4; ++i) {
        const int stride = params.value(stride_names[i], 0);
        if (stride) {
            strides[i] = stride;
            planes[i]  = base + params.value(offset_names[i], size_t(0));
        }
    }
    if (!planes[0] || !planes[1])
        return {};
    auto *output = reinterpret_cast<uint8_t *>(rgb.data());
    const std::array<int, 1> output_stride{width * 3 * 2};
    if (sws_scale(
            sws_context_,
            planes.data(),
            strides.data(),
            0,
            dims.y,
            &output,
            output_stride.data()) <= 0)
        return {};

    auto *dst = reinterpret_cast<float *>(thumbnail->data().data());
    for (const uint16_t sample : rgb)
        *(dst++) = float(sample) / 65535.0f;
    return thumbnail;
}

} // namespace xstudio::media_reader::ffmpeg
