// SPDX-License-Identifier: Apache-2.0
#include <filesystem>
#include <caf/actor_registry.hpp>

#include "xstudio/ui/qt/offscreen_viewport_base.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/playhead/playhead_actor.hpp"
#include "xstudio/thumbnail/enums.hpp"

#include <ImfRgbaFile.h>
#include <vector>
#include <QStringList>
#include <QWidget>
#include <QByteArray>
#include <QImageWriter>
#include <QThread>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickGraphicsDevice>
#include <QFontDatabase>


using namespace caf;
using namespace xstudio;
using namespace xstudio::ui;
using namespace xstudio::ui::qt;
using namespace xstudio::ui::viewport;
using namespace xstudio::ui::qml;

namespace fs = std::filesystem;


OffscreenViewportBase::OffscreenViewportBase(const std::string name, bool sync_with_main_viewports)
    : super() {

    // This class is a QObject with a caf::actor 'companion' that allows it
    // to receive and send caf messages - here we run necessary initialisation
    // of the companion actor
    super::init(xstudio::ui::qml::CafSystemObject::get_actor_system());

    scoped_actor sys{xstudio::ui::qml::CafSystemObject::get_actor_system()};

    // Now we create our OpenGL xSTudio viewport - this has 'Viewport(Module)' as
    // its base class that provides various caf message handlers that are added
    // to our companion actor's 'behaviour' to create a fully functioning
    // viewport that can receive caf messages including framebuffers and also
    // to render the viewport into our GLContext
    utility::JsonStore jsn;
    jsn["base"]       = utility::JsonStore();
    jsn["window_id"]  = name;
    xstudio_viewport_ = new Viewport(jsn, as_actor(), sync_with_main_viewports, name);

    /* Provide a callback so the Viewport can tell this class when some property of the viewport
    has changed and such events can be propagated to other QT components, for example */
    auto callback = [this](auto &&PH1) {
        receive_change_notification(std::forward<decltype(PH1)>(PH1));
    };
    xstudio_viewport_->set_change_callback(callback);

    // join studio events, so we know when a new session has been created
    auto grp = utility::request_receive<caf::actor>(
        *sys,
        system().registry().template get<caf::actor>(studio_registry),
        utility::get_event_group_atom_v);

    utility::request_receive<bool>(*sys, grp, broadcast::join_broadcast_atom_v, as_actor());

    session_actor_addr_ = actorToQString(
        system(),
        utility::request_receive<caf::actor>(
            *sys,
            system().registry().template get<caf::actor>(studio_registry),
            session::session_atom_v));

    // Here we set-up the caf message handler for this class by combining the
    // message handler from OpenGLViewportRenderer with our own message handlers for offscreen
    // rendering
    set_message_handler([=](caf::actor_companion * /*self*/) -> caf::message_handler {
        return xstudio_viewport_->message_handler().or_else(
            caf::message_handler{

                // insert additional message handlers here
                [=](render_viewport_to_image_atom, const int width, const int height)
                    -> result<bool> {
                    try {
                        // copies a QImage to the Clipboard
                        renderSnapshot(width, height);
                        return true;
                    } catch (std::exception &e) {
                        return caf::make_error(xstudio_error::error, e.what());
                    }
                },

                [=](render_viewport_to_image_atom,
                    const caf::uri path,
                    const int width,
                    const int height) -> result<bool> {
                    try {
                        renderSnapshot(width, height, path);
                        return true;
                    } catch (std::exception &e) {
                        return caf::make_error(xstudio_error::error, e.what());
                    }
                },

                [=](render_viewport_to_image_atom,
                    const thumbnail::THUMBNAIL_FORMAT format,
                    const int width,
                    const int height) -> result<thumbnail::ThumbnailBufferPtr> {
                    try {
                        return renderToThumbnail(format, width, height);
                    } catch (std::exception &e) {
                        return caf::make_error(xstudio_error::error, e.what());
                    }
                },

                [=](render_viewport_to_image_atom,
                    caf::actor media_actor,
                    const int media_frame,
                    const thumbnail::THUMBNAIL_FORMAT format,
                    const int width,
                    const bool auto_scale,
                    const bool show_annotations) -> result<thumbnail::ThumbnailBufferPtr> {
                    thumbnail::ThumbnailBufferPtr r;
                    try {
                        r = renderMediaFrameToThumbnail(
                            media_actor,
                            media_frame,
                            format,
                            width,
                            auto_scale,
                            show_annotations);
                    } catch (std::exception &e) {
                        return caf::make_error(xstudio_error::error, e.what());
                    }
                    return r;
                },

                [=](render_viewport_to_image_atom,
                    caf::actor media_actor,
                    const int media_frame,
                    const int width,
                    const int height,
                    const caf::uri path) -> result<bool> {
                    try {

                        media_reader::ImageBufPtr image =
                            renderMediaFrameToImage(media_actor, media_frame, width, height);
                        auto p = fs::path(xstudio::utility::uri_to_posix_path(path));

                        std::string ext = xstudio::utility::ltrim_char(
#ifdef _WIN32
                            xstudio::utility::to_upper_path(p.extension()),
#else
                            xstudio::utility::to_upper(p.extension()),
#endif
                            '.'); // yuk!

                        if (ext == "EXR") {
                            this->exportToEXR(image, path);
                        } else {
                            this->exportToCompressedFormat(image, path, ext);
                        }

                    } catch (std::exception &e) {
                        // spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
                        return caf::make_error(xstudio_error::error, e.what());
                    }
                    return true;
                },

                [=](render_viewport_to_image_atom,
                    caf::actor media_actor,
                    const timebase::flicks playhead_timepoint,
                    const thumbnail::THUMBNAIL_FORMAT format,
                    const int width,
                    const bool auto_scale,
                    const bool show_annotations) -> result<thumbnail::ThumbnailBufferPtr> {
                    thumbnail::ThumbnailBufferPtr r;
                    try {
                        r = renderMediaFrameToThumbnail(
                            media_actor,
                            playhead_timepoint,
                            format,
                            width,
                            auto_scale,
                            show_annotations);
                    } catch (std::exception &e) {
                        return caf::make_error(xstudio_error::error, e.what());
                    }
                    return r;
                },

                [=](video_output_actor_atom,
                    caf::actor video_output_actor,
                    int outputWidth,
                    int outputHeight,
                    ImageFormat format) {
                    video_output_actor_ = video_output_actor;
                    vid_out_width_      = outputWidth;
                    vid_out_height_     = outputHeight;
                    vid_out_format_     = format;
                },

                [=](video_output_actor_atom, caf::actor video_output_actor) {
                    video_output_actor_ = video_output_actor;
                },

                [=](render_viewport_to_image_atom,
                    const int width,
                    const int height,
                    ImageFormat format) -> result<media_reader::ImageBufPtr> {
                    media_reader::ImageBufPtr new_frame;
                    try {

                        renderToImageBuffer(width, height, new_frame, format, true);

                    } catch (std::exception &e) {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
                    }

                    return new_frame;
                },

                [=](render_viewport_to_image_atom,
                    const utility::time_point &tp,
                    const bool return_frame,
                    const bool skip_if_out_of_date) {
                    // force a redraw
                    if (video_output_actor_) {

                        if (return_frame) {

                            if (last_rendered_frame_ && !xstudio_viewport_->playing()) {
                                // no need to re-render if Redraw callback hasn't
                                // arrived since we last rendered
                                anon_mail(last_rendered_frame_).send(video_output_actor_);

                            } else {

                                media_reader::ImageBufPtr new_frame;
                                try {
                                    renderToImageBuffer(
                                        vid_out_width_,
                                        vid_out_height_,
                                        new_frame,
                                        vid_out_format_,
                                        false,
                                        tp);
                                } catch (std::exception &e) {
                                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
                                }
                                anon_mail(new_frame).send(video_output_actor_);
                                last_rendered_frame_ = new_frame;
                            }

                        } else {

                            try {

                                auto msec_lag =
                                    std::chrono::duration_cast<std::chrono::milliseconds>(
                                        tp - utility::clock::now())
                                        .count();
                                if (skip_if_out_of_date && msec_lag < -100) {
                                    // we've requested to render a frame IN THE PAST ... this
                                    // means we are unable to render fast enough to keep up with
                                    // render requests so we will skip the render
                                } else {
                                    render(
                                        vid_out_width_,
                                        vid_out_height_,
                                        vid_out_format_,
                                        false,
                                        tp);
                                }

                                // we still return an empty frame to the video output plugin
                                // so it can
                                anon_mail(media_reader::ImageBufPtr())
                                    .send(video_output_actor_);

                            } catch (std::exception &e) {

                                spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
                            }
                        }
                    }
                },

                [=](render_viewport_to_image_atom,
                    caf::actor media_actor,
                    const int media_frame,
                    const bool include_image,
                    const bool include_overlays,
                    const bool include_drawings,
                    const int width,
                    const int height,
                    const caf::uri &output) -> result<bool> {
                    try {

                        media_reader::ImageBufPtr image = renderMediaFrameToImage(
                            media_actor,
                            media_frame,
                            width,
                            height,
                            include_image,
                            include_overlays,
                            include_drawings);
                        auto p          = fs::path(xstudio::utility::uri_to_posix_path(output));
                        std::string ext = xstudio::utility::ltrim_char(
#ifdef _WIN32
                            xstudio::utility::to_upper_path(p.extension()),
#else
                            xstudio::utility::to_upper(p.extension()),
#endif
                            '.'); // yuk!

                        if (ext == "EXR") {
                            this->exportToEXR(image, output);
                        } else {
                            this->exportToCompressedFormat(image, output, ext, !include_image);
                        }

                    } catch (std::exception &e) {
                        return caf::make_error(xstudio_error::error, e.what());
                    }
                    return true;
                },

                // event coming from session actor
                [=](utility::event_atom, session::session_atom, caf::actor session) {
                    session_actor_addr_ = actorToQString(system(), session);
                },

                // event coming from session actor (ignore)
                [=](utility::event_atom,
                    session::session_request_atom,
                    const std::string &path,
                    const utility::JsonStore &js) {},

                // sets a custom frame grabber that takes over the GPU routine that
                // takes the viewport framebuffer and returns an ImageBufPtr - used
                // by video output plugins
                [=](ViewportFramePostProcessorPtr custom_post_draw_hook) -> bool {
                    post_draw_hook_ = custom_post_draw_hook;
                    return true;
                }

            });
    });
}

void OffscreenViewportBase::sceneChanged() { last_rendered_frame_.reset(); }

void OffscreenViewportBase::renderSnapshot(const int width, const int height, const caf::uri path) {

    // temp hack - put in a 500ms delay so the playhead can update the
    // annotations plugin with the annotations data.
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));


    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Invalid image dimensions.");
    }

    media_reader::ImageBufPtr image(new media_reader::ImageBuffer());

    renderToImageBuffer(width, height, image, ImageFormat::RGBA_16F, true);

    if (path.empty()) {
        // we can call this with empty path - image is copied to clipboard
        this->exportToCompressedFormat(image, path, "");
    } else {

        auto p = fs::path(xstudio::utility::uri_to_posix_path(path));

        std::string ext = xstudio::utility::ltrim_char(
#ifdef _WIN32
            xstudio::utility::to_upper_path(p.extension()),
#else
            xstudio::utility::to_upper(p.extension()),
#endif
            '.'); // yuk!

        if (ext == "EXR") {
            this->exportToEXR(image, path);
        } else {
            this->exportToCompressedFormat(image, path, ext);
        }
    }
}

void OffscreenViewportBase::setPlayhead(const QString &playheadAddress) {

    try {

        scoped_actor sys{as_actor()->home_system()};
        auto playhead_actor = qml::actorFromQString(as_actor()->home_system(), playheadAddress);

        if (playhead_actor) {
            xstudio_viewport_->set_playhead(playhead_actor);

            if (xstudio_viewport_->colour_pipeline()) {
                // get the current on screen media source
                auto media_source = utility::request_receive<utility::UuidActor>(
                    *sys, playhead_actor, playhead::media_source_atom_v, true);

                // update the colour pipeline with the media source so it can
                // run its logic to update the view/display attributes etc.
                utility::request_receive<bool>(
                    *sys,
                    xstudio_viewport_->colour_pipeline(),
                    playhead::media_source_atom_v,
                    media_source);
            }
        }


    } catch (std::exception &e) {
        spdlog::warn("{} {} ", __PRETTY_FUNCTION__, e.what());
    }
}

void OffscreenViewportBase::exportToEXR(const media_reader::ImageBufPtr &buf, const caf::uri path) {
    Imf::Header header;
    const Imath::V2i dim = buf->image_size_in_pixels();
    Imath::Box2i box;
    box.min.x           = 0;
    box.min.y           = 0;
    box.max.x           = dim.x - 1;
    box.max.y           = dim.y - 1;
    header.dataWindow() = header.displayWindow() = box;
    header.compression()                         = Imf::PIZ_COMPRESSION;
    Imf::RgbaOutputFile outFile(utility::uri_to_posix_path(path).c_str(), header);
    Imf::Rgba *bptr = (Imf::Rgba *)buf->buffer();
    bptr += (dim.y - 1) * dim.x; // move to final scanline
    outFile.setFrameBuffer(
        bptr,
        1,     // pix stride
        -dim.x // line stride (i.e.) step backwards through buffer
    );
    outFile.writePixels(dim.y);
}

void OffscreenViewportBase::exportToCompressedFormat(
    const media_reader::ImageBufPtr &buf,
    const caf::uri path,
    const std::string &ext,
    const bool has_alpha) {

    if (has_alpha) {

        QImage im(
            buf->image_size_in_pixels().x,
            buf->image_size_in_pixels().y,
            QImage::Format_RGBA16FPx4);
        const size_t scanline_width = buf->image_size_in_pixels().x * 4;
        const half *src             = reinterpret_cast<const half *>(buf->buffer());
        src += (buf->image_size_in_pixels().y - 1) * buf->image_size_in_pixels().x *
               4; // jump to last scanline

        for (int y = 0; y < buf->image_size_in_pixels().y; y++) {

            memcpy(im.scanLine(y), src, scanline_width * sizeof(half));
            src -= scanline_width;
        }

        QImageWriter writer(xstudio::utility::uri_to_posix_path(path).c_str());
        if (!writer.write(im)) {
            throw std::runtime_error(writer.errorString().toStdString().c_str());
        }

    } else {

        thumbnail::ThumbnailBufferPtr r = rgb96thumbFromHalfFloatImage(buf);
        r->convert_to(thumbnail::TF_RGB24);

        // N.B. We can't pass our thumnail buffer directly to QImage constructor as
        // it requires 32 bit alignment on scanlines and our Thumbnail buffer is
        // not designed as such.

        const int width  = r->width();
        const int height = r->height();

        const auto *in_px = (const uint8_t *)r->data().data();
        QImage im(width, height, QImage::Format_RGB888);

        // In fact QImage is a bit funky and won't let us write whole scanlines so
        // have to do it pixel by pixel
        for (int line = 0; line < height; line++) {
            for (int x = 0; x < width; x++) {
                im.setPixelColor(x, line, QColor((int)in_px[0], (int)in_px[1], (int)in_px[2]));
                in_px += 3;
            }
        }

        /*int compLevel =
            ext == "TIF" || ext == "TIFF" ? std::max(compression, 1) : (10 - compression) *
           10;*/
        // TODO : check m_filePath for extension, if not, add to it. Do it on QML side after
        // merging with new UI branch

        if (path.empty()) {
            QApplication::clipboard()->setImage(im, QClipboard::Clipboard);
            return;
        }

        QImageWriter writer(xstudio::utility::uri_to_posix_path(path).c_str());
        // writer.setCompression(compLevel);
        if (!writer.write(im)) {
            throw std::runtime_error(writer.errorString().toStdString().c_str());
        }
    }
}

void OffscreenViewportBase::receive_change_notification(Viewport::ChangeCallbackId id) {

    // something has changed that will affect the rendered output. clear
    // last_rendered_frame_
    last_rendered_frame_.reset();
}

void OffscreenViewportBase::make_conversion_lut() {

    if (half_to_int_32_lut_.empty()) {
        auto int_max = double(std::numeric_limits<uint32_t>::max());
        half_to_int_32_lut_.resize(1 << 16);
        for (size_t i = 0; i < (1 << 16); ++i) {
            half h;
            h.setBits(i);
            half_to_int_32_lut_[i] =
                uint32_t(round(std::max(0.0, std::min(1.0, double(h))) * int_max));
        }
    }
}

thumbnail::ThumbnailBufferPtr
    OffscreenViewportBase::rgb96thumbFromHalfFloatImage(const media_reader::ImageBufPtr &image) {

    const Imath::V2i image_size = image->image_size_in_pixels();

    // since we only run this routine ourselves and set-up the image properly
    // this mismatch can't happen but check anyway just in case. Due to padding
    // image buffers are usually a bit larger than the tight pixel size.
    size_t expected_size = image_size.x * image_size.y * sizeof(half) * 4;
    if (expected_size > image->size()) {

        std::string err(
            fmt::format(
                "{} Image buffer size of {} does not agree with image pixels size of {} "
                "({}x{}).",
                __PRETTY_FUNCTION__,
                image->size(),
                expected_size,
                image_size.x,
                image_size.y));
        throw std::runtime_error(err.c_str());
    }


    // init RGBA float array
    thumbnail::ThumbnailBufferPtr r(
        new thumbnail::ThumbnailBuffer(image_size.x, image_size.y, thumbnail::TF_RGBF96));

    // note 'image' is (probably) already in a display space. The offscreen
    // viewport has its own instance of ColourPipeline plugin doing the colour
    // management. So our colours are normalised to 0-1 range.

    make_conversion_lut();

    const half *in = (half *)image->buffer();
    float *out     = (float *)r->data().data();
    size_t sz      = image_size.x * image_size.y;
    while (sz--) {
        *(out++) = *(in++);
        *(out++) = *(in++);
        *(out++) = *(in++);
        in++; // skip alpha
    }

    r->flip();

    return r;
}

thumbnail::ThumbnailBufferPtr OffscreenViewportBase::renderToThumbnail(
    const thumbnail::THUMBNAIL_FORMAT format,
    const int width,
    const bool auto_scale,
    const bool show_annotations) {

    media_reader::ImageBufPtr image = xstudio_viewport_->get_onscreen_image();

    if (!image) {
        std::string err(
            fmt::format(
                "{} Failed to pull images to offscreen renderer.", __PRETTY_FUNCTION__));
        throw std::runtime_error(err.c_str());
    }

    const Imath::V2i image_dims = image->image_size_in_pixels();
    if (image_dims.x <= 0 || image_dims.y <= 0) {
        std::string err(fmt::format("{} Null image in viewport.", __PRETTY_FUNCTION__));
        throw std::runtime_error(err.c_str());
    }

    float effective_image_height = float(image_dims.y) / image.frame_id().pixel_aspect();

    if (width <= 0 || auto_scale) {
        xstudio_viewport_->set_fit_mode(FitMode::One2One);
        return renderToThumbnail(format, image_dims.x, int(round(effective_image_height)));
    } else {
        xstudio_viewport_->set_fit_mode(FitMode::Best);
        return renderToThumbnail(
            format, width, int(round(width * effective_image_height / image_dims.x)));
    }
}

thumbnail::ThumbnailBufferPtr OffscreenViewportBase::renderToThumbnail(
    const thumbnail::THUMBNAIL_FORMAT format, const int width, const int height) {

    media_reader::ImageBufPtr image = renderToImageBuf(width, height, true, false, true);
    thumbnail::ThumbnailBufferPtr r = rgb96thumbFromHalfFloatImage(image);
    r->convert_to(format);
    return r;
}

media_reader::ImageBufPtr OffscreenViewportBase::renderToImageBuf(
    int width,
    int height,
    const bool include_image,
    const bool include_overlays,
    const bool include_drawings) {

    media_reader::ImageBufPtr image2 = xstudio_viewport_->get_onscreen_image();
    if (!image2) {
        std::string err(
            fmt::format(
                "{} Failed to pull images to offscreen renderer.", __PRETTY_FUNCTION__));
        throw std::runtime_error(err.c_str());
    }
    media_reader::ImageBufPtr image(new media_reader::ImageBuffer());

    if (width <= 0 || height <= 0) {
        // match output image size to the on-screen image (media image) size
        width  = image2->image_size_in_pixels().x;
        height = image2->image_size_in_pixels().y;
    }

    if (!include_image) {
        // make the image to be rendered actually transparent
        image2.set_invisible(true);
    }

    renderToImageBuffer(
        width,
        height,
        image,
        ImageFormat::RGBA_16F,
        true,
        utility::clock::now(),
        image2,
        include_overlays,
        include_drawings);
    return image;
}

media_reader::ImageBufPtr OffscreenViewportBase::renderMediaFrameToImage(
    caf::actor media_actor,
    const int media_frame,
    const int width,
    const int height,
    const bool include_image,
    const bool include_overlays,
    const bool include_drawings) {

    if (!local_playhead_) {
        auto a = caf::actor_cast<caf::event_based_actor *>(as_actor());
        local_playhead_ =
            a->spawn<playhead::PlayheadActor>("Offscreen Viewport Local Playhead");

        a->link_to(local_playhead_);
    }
    // first, set the local playhead to be our image source
    xstudio_viewport_->set_playhead(local_playhead_);

    scoped_actor sys{as_actor()->home_system()};

    // now set the media source on the local playhead
    utility::request_receive<bool>(
        *sys, local_playhead_, playhead::source_atom_v, std::vector<caf::actor>({media_actor}));

    // now move the playhead to requested frame
    utility::request_receive<bool>(*sys, local_playhead_, playhead::jump_atom_v, media_frame);

    return renderToImageBuf(width, height, include_image, include_overlays, include_drawings);
}

thumbnail::ThumbnailBufferPtr OffscreenViewportBase::renderMediaFrameToThumbnail(
    caf::actor media_actor,
    const int media_frame,
    const thumbnail::THUMBNAIL_FORMAT format,
    const int width,
    const bool auto_scale,
    const bool show_annotations) {

    if (!local_playhead_) {
        auto a = caf::actor_cast<caf::event_based_actor *>(as_actor());
        local_playhead_ =
            a->spawn<playhead::PlayheadActor>("Offscreen Viewport Local Playhead");

        a->link_to(local_playhead_);
    }
    // first, set the local playhead to be our image source
    xstudio_viewport_->set_playhead(local_playhead_);

    scoped_actor sys{as_actor()->home_system()};

    // now set the media source on the local playhead
    utility::request_receive<bool>(
        *sys, local_playhead_, playhead::source_atom_v, std::vector<caf::actor>({media_actor}));

    // now move the playhead to requested frame
    utility::request_receive<bool>(*sys, local_playhead_, playhead::jump_atom_v, media_frame);

    return renderToThumbnail(format, width, auto_scale, show_annotations);
}

thumbnail::ThumbnailBufferPtr OffscreenViewportBase::renderMediaFrameToThumbnail(
    caf::actor media_actor,
    const timebase::flicks playhead_position_flicks,
    const thumbnail::THUMBNAIL_FORMAT format,
    const int width,
    const bool auto_scale,
    const bool show_annotations) {
    if (!local_playhead_) {
        auto a = caf::actor_cast<caf::event_based_actor *>(as_actor());
        local_playhead_ =
            a->spawn<playhead::PlayheadActor>("Offscreen Viewport Local Playhead");
        a->link_to(local_playhead_);
    }

    // first, set the local playhead to be our image source
    xstudio_viewport_->set_playhead(local_playhead_);

    scoped_actor sys{as_actor()->home_system()};

    // now set the media source on the local playhead
    utility::request_receive<bool>(
        *sys, local_playhead_, playhead::source_atom_v, std::vector<caf::actor>({media_actor}));

    // now move the playhead to requested frame
    utility::request_receive<bool>(
        *sys, local_playhead_, playhead::jump_atom_v, playhead_position_flicks);

    return renderToThumbnail(format, width, auto_scale, show_annotations);
}

void OffscreenViewportBase::renderToImageBuffer(
    const int w,
    const int h,
    media_reader::ImageBufPtr &destination_image,
    const ImageFormat format,
    const bool sync_fetch_playhead_image,
    const utility::time_point &tp,
    const media_reader::ImageBufPtr &image_to_use,
    const bool include_overlays,
    const bool include_drawings) {
    // auto t0 = utility::clock::now();

    // the actual render call
    render(
        w,
        h,
        format,
        sync_fetch_playhead_image,
        tp,
        image_to_use,
        include_overlays,
        include_drawings);

    capture_framebuffer(w, h, format, destination_image);

}
