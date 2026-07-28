// SPDX-License-Identifier: Apache-2.0
#include "xstudio/media_reader/media_reader.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/logging.hpp"
#include <chrono>
#include "demo_media_reader_pixel_unpack_shader.hpp"
#include "demo_media_reader.hpp"

using namespace xstudio;
using namespace xstudio::global_store;
using namespace xstudio::media_reader;
using namespace xstudio::utility;
using namespace xstudio::demo_plugin;

namespace {

#ifdef _WIN32

void srand48(const size_t h) {
    // assume bottom 4 bytes of h are
    // good enough to do what I need here
    srand((unsigned int)(h));
}

double drand48() { return (double)rand() / double(RAND_MAX); }

#endif

static Uuid shader_uuid{"241b2d6a-8107-4944-96da-ead12dc26c42"};

static const Imath::V2i _image_size(1920, 1080);

std::vector<float> palette(const std::string &seed) {

    auto hasher = std::hash<std::string>();
    size_t hash = hasher(seed);

    srand48(hash);

    std::vector<float> _p;
    _p.resize(1024 * 4);
    for (int i = 0; i < 1024; ++i) {
        _p[i * 4]     = drand48();
        _p[i * 4 + 1] = drand48();
        _p[i * 4 + 2] = drand48();
        _p[i * 4 + 3] = 1.0f;
    }
    _p[4000] = 0.0;
    _p[4001] = 0.0;
    _p[4002] = 0.0;
    _p[4003] = 0.0;
    return _p;
}

static std::mutex __mutex;

} // namespace

ProceduralImageGenReader::ProceduralImageGenReader(const utility::JsonStore &prefs)
    : MediaReader("ProceduralImageGen", prefs) 
{

    pixel_unpack_shader_.reset(new DemoPixelUnpackShader(shader_uuid));

}

ImageBufPtr ProceduralImageGenReader::image(const media::AVFrameID &mptr) {

    const auto uri_string = to_string(mptr.uri());

    // DebugTimer dd(path);

    // our data source plugin adds a proxy source to the main media item(s).
    // The only difference is that the media URI has .proxy. just before the suffix.
    const bool is_proxy = uri_string.find(".proxy.") != std::string::npos;

    // Because the frames are procedurally generated we decide on the dimensions
    // here, but of course most REAL media will define its own size in the
    // file encoding itself.
    const int width  = _image_size.x / (is_proxy ? 4 : 1);
    const int height = _image_size.y / (is_proxy ? 4 : 1);

    JsonStore jsn;
    jsn["width"]  = width;
    jsn["height"] = height;

    ImageBufPtr buf(new ImageBuffer(jsn));

    const float scale = 1.0f + float(mptr.frame()) / 100.0f;

    const std::string uri_stem(uri_string, 0, uri_string.find("."));
    __mutex.lock();
    const std::vector<float> p = palette(uri_stem);

    // float RGBA mandlebrot image. THis is how we would fill an image buffer
    // with pixel data read from an image or stream on the filesystem.
    // We don't use this in the example after all and instead use the GPU
    // to generate pixel data for us at draw time.
    /*auto b = reinterpret_cast<float *>(buf->allocate(width*height*4*sizeof(float)));
    const int max_iteration = 50;

    for (int ypix = 0; ypix < height; ++ypix) {
        const float y0 = (-1.12f + float(ypix)*2.24f/float(height))*scale;
        for (int xpix = 0; xpix < width; ++xpix) {
            const float x0 = (-2.0f + float(xpix)*2.47f/float(width))*scale;
            float x = 0.0f;
            float y = 0.0f;
            int iteration = 0;
            while ( (x*x + y*y) < 4.0f && iteration < max_iteration) {
                float xtemp = x*x - y*y + x0;
                y = 2*x*y + y0;
                x = xtemp;
                iteration++;
            }
            if (iteration == max_iteration) {
              memset(b, 0, 4 * sizeof(float));
            } else {
              memcpy(b, p.data()+iteration*4, 4 * sizeof(float));
            }
            b+=4;
        }
    }*/

    // These are the shader params we need to set-up our pixel unpack shader
    // (the shader code at the top of this file).

    // Note that the name of the image source is used to seed the random
    // number generator and therefore we get consistent colours for a
    // given URL.
    utility::JsonStore shader_params;
    shader_params["iTime"] =
        drand48() * 50.0f + float(mptr.frame() / (10.0f + drand48() * 30.0f));
    shader_params["width"]   = width;
    shader_params["height"]  = height;
    shader_params["colour1"] = utility::ColourTriplet(
        0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8);
    shader_params["colour2"] = utility::ColourTriplet(
        0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8);
    shader_params["colour3"] = utility::ColourTriplet(
        0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8);
    shader_params["colour4"] = utility::ColourTriplet(
        0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8);
    shader_params["colour5"] = utility::ColourTriplet(
        0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8);
    shader_params["colour6"] = utility::ColourTriplet(
        0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8, 0.1 + drand48() * 0.8);
    shader_params["iMouse"] =
        Imath::V4f(drand48() * width, drand48() * height, drand48() < 0.5 ? -1.0 : 1.0, 0.0f);
    __mutex.unlock();

    buf->set_shader_params(shader_params);

    // Here we can set additional metadata for the frame. Within reason you can put
    // any data you like in here. It can then be used in, for example, the Media Metadata
    // HUD to draw over the image in the xSTUDIO viewport.
    utility::JsonStore frame_metadata;
    buf->set_metadata(frame_metadata);

    // here we set the shader on the image
    buf->set_shader(pixel_unpack_shader_);

    buf->set_image_dimensions(
        Imath::V2i(width, height), // image DISPLAY width & height
        Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(width, height)) // image data window area
    );

    buf->params()["path"]      = to_string(mptr.uri());
    buf->params()["stream_id"] = mptr.stream_id();


    return buf;
}

MRCertainty
ProceduralImageGenReader::supported(const caf::uri &uri, const std::array<uint8_t, 16> &sig) {
    const auto uri_string = to_string(uri);

    // note that 'sig' is the first 16 bytes of the image. Some formats like JPEG and EXR
    // have a magic number that we can check against.

    // we load when uri ends in '.fake'
    if (uri_string.find(".fake") == uri_string.size() - 5)
        return MRC_FORCE;

    return MRC_NO;
}

xstudio::media::MediaDetail ProceduralImageGenReader::detail(const caf::uri &uri) const {

    // Here we provide information on the streams (also equivalent to layers or parts in
    // an image). Streams can be image or audio.

    const auto uri_string = to_string(uri);
    utility::Timecode tc("00:00:00:00");

    // For our example we have 'encoded' the frame range for these virtual files
    // in their URL, which is of course a hack because there is no real resource
    // to read a frame range from.
    int first_frame = 1001;
    int last_frame  = 1051;
    std::cmatch m;
    const std::regex frame_range("\\.([0-9]+)\\-([0-9]+)\\.fake$");
    if (std::regex_search(uri_string.c_str(), m, frame_range)) {
        try {
            first_frame = std::stoi(m[1].str());
            last_frame  = std::stoi(m[2].str());
        } catch (...) {
        }
    }

    utility::FrameRateDuration frd(
        last_frame - first_frame + 1, utility::FrameRate(timebase::k_flicks_24fps));

    tc = utility::Timecode(first_frame, 24.0);

    std::vector<media::StreamDetail> streams;

    // set the res, depending of 'proxy' is in the URI string
    const bool is_proxy = uri_string.find(".proxy.") != std::string::npos;
    const int width     = _image_size.x / (is_proxy ? 4 : 1);
    const int height    = _image_size.y / (is_proxy ? 4 : 1);

    streams.emplace_back(media::StreamDetail(frd, "RGBA")); // stream name = RGBA
    streams.back().resolution_   = Imath::V2f(width, height);
    streams.back().pixel_aspect_ = 1.0f;
    streams.back().index_        = 0;

    return xstudio::media::MediaDetail(name(), streams, tc);
}

thumbnail::ThumbnailBufferPtr
ProceduralImageGenReader::thumbnail(const media::AVFrameID &mptr, const size_t thumb_size) {

    // by returning a null result, this forces xSTUDIO into fallback behavior where it will
    // load the fullsize image and then render it (via the offscreen_viewport) to convert
    // to the desired thumbnail format.
    return thumbnail::ThumbnailBufferPtr();
}
