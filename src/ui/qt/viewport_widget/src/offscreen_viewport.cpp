// SPDX-License-Identifier: Apache-2.0
#include <GL/glew.h>
#include <GL/gl.h>

#include <filesystem>


#include "xstudio/ui/qt/offscreen_viewport.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"
#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"
#include "xstudio/playhead/playhead_actor.hpp"
#include "xstudio/thumbnail/enums.hpp"

#include <ImfRgbaFile.h>
#include <vector>
#include <QStringList>
#include <QWidget>
#include <QByteArray>
#include <QImageWriter>
#include <QThread>

using namespace caf;
using namespace xstudio;
using namespace xstudio::ui;
using namespace xstudio::ui::qt;

namespace fs = std::filesystem;

OffscreenViewport::OffscreenViewport() : super() {

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
    static int offscreen_idx = -1;
    utility::JsonStore jsn;
    jsn["base"]        = utility::JsonStore();
    viewport_renderer_ = new ui::viewport::Viewport(
        jsn,
        as_actor(),
        offscreen_idx--,
        ui::viewport::ViewportRendererPtr(new opengl::OpenGLViewportRenderer(true, false)));

    /* Provide a callback so the Viewport can tell this class when some property of the viewport
    has changed and such events can be propagated to other QT components, for example */
    auto callback = [this](auto &&PH1) {
        receive_change_notification(std::forward<decltype(PH1)>(PH1));
    };
    viewport_renderer_->set_change_callback(callback);

    self()->set_down_handler([=](down_msg &msg) {
        if (msg.source == video_output_actor_) {
            video_output_actor_ = caf::actor();
        }
    });

    // Here we set-up the caf message handler for this class by combining the
    // message handler from OpenGLViewportRenderer with our own message handlers for offscreen
    // rendering
    set_message_handler([=](caf::actor_companion * /*self*/) -> caf::message_handler {
        return viewport_renderer_->message_handler().or_else(caf::message_handler{

            // insert additional message handlers here
            [=](viewport::render_viewport_to_image_atom, const int width, const int height)
                -> result<bool> {
                try {
                    // copies a QImage to the Clipboard
                    renderSnapshot(width, height);
                    return true;
                } catch (std::exception &e) {
                    return caf::make_error(xstudio_error::error, e.what());
                }
            },

            [=](viewport::render_viewport_to_image_atom,
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

            [=](viewport::render_viewport_to_image_atom,
                const thumbnail::THUMBNAIL_FORMAT format,
                const int width,
                const int height) -> result<thumbnail::ThumbnailBufferPtr> {
                try {
                    return renderToThumbnail(format, width, height);
                } catch (std::exception &e) {
                    return caf::make_error(xstudio_error::error, e.what());
                }
            },

            [=](viewport::render_viewport_to_image_atom,
                caf::actor media_actor,
                const int media_frame,
                const thumbnail::THUMBNAIL_FORMAT format,
                const int width,
                const bool auto_scale,
                const bool show_annotations) -> result<thumbnail::ThumbnailBufferPtr> {
                thumbnail::ThumbnailBufferPtr r;
                try {
                    r = renderMediaFrameToThumbnail(
                        media_actor, media_frame, format, width, auto_scale, show_annotations);
                } catch (std::exception &e) {
                    return caf::make_error(xstudio_error::error, e.what());
                }
                return r;
            },

            [=](viewport::render_viewport_to_image_atom,
                caf::actor media_actor,
                const timebase::flicks playhead_timepoint,
                const thumbnail::THUMBNAIL_FORMAT format,
                const int width,
                const bool auto_scale,
                const bool show_annotations) -> result<thumbnail::ThumbnailBufferPtr> {
                thumbnail::ThumbnailBufferPtr r;
                try {
                    r = renderMediaFrameToThumbnail(
                        media_actor, playhead_timepoint, format, width, auto_scale, show_annotations);
                } catch (std::exception &e) {
                    return caf::make_error(xstudio_error::error, e.what());
                }
                return r;
            }});
    });

    initGL();
}

OffscreenViewport::~OffscreenViewport() {

    gl_context_->makeCurrent(surface_);
    delete viewport_renderer_;
    glDeleteTextures(1, &texId_);
    glDeleteFramebuffers(1, &fboId_);
    glDeleteTextures(1, &depth_texId_);

    delete gl_context_;
    delete surface_;

    video_output_actor_ = caf::actor();
}

void OffscreenViewport::initGL() {

    if (!gl_context_) {
        // create our own GL context
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        format.setDepthBufferSize(24);
        format.setRedBufferSize(8);
        format.setGreenBufferSize(8);
        format.setBlueBufferSize(8);
        format.setAlphaBufferSize(8);
        format.setRenderableType(QSurfaceFormat::OpenGL);

        gl_context_ = new QOpenGLContext(nullptr); // m_window->openglContext();
        gl_context_->setFormat(format);
        if (!gl_context_)
            throw std::runtime_error(
                "OffscreenViewport::initGL - could not create QOpenGLContext.");
        if (!gl_context_->create()) {
            throw std::runtime_error("OffscreenViewport::initGL - failed to creat GL Context "
                                     "for offscreen rendering.");
        }

        // we also require a QSurface to use the GL context
        surface_ = new QOffscreenSurface(nullptr, nullptr);
        surface_->setFormat(format);
        surface_->create();

        // gl_context_->makeCurrent(surface_);

        // we also require a QSurface to use the GL context
        surface_ = new QOffscreenSurface(nullptr, nullptr);
        surface_->setFormat(format);
        surface_->create();

        thread_ = new QThread();
        gl_context_->moveToThread(thread_);
        moveToThread(thread_);
        thread_->start();
    }
}

void OffscreenViewport::renderSnapshot(const int width, const int height, const caf::uri path) {

    initGL();

    // temp hack - put in a 500ms delay so the playhead can update the
    // annotations plugin with the annotations data.
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (path.empty()) {
        throw std::runtime_error("Invalid (empty) file path.");
    }

    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Invalid image dimensions.");
    }

    media_reader::ImageBufPtr image(new media_reader::ImageBuffer());
    renderToRGBAHalf16ImageBuffer(width, height, image);

    auto p = fs::path(xstudio::utility::uri_to_posix_path(path));

    std::string ext = xstudio::utility::ltrim_char(
        xstudio::utility::to_upper(p.extension()),
        '.'); // yuk!

    if (ext == "EXR") {
        this->exportToEXR(image, path);
    } else {
        this->exportToCompressedFormat(image, path, ext);
    }
}

void OffscreenViewport::setPlayhead(const QString &playheadAddress) {

    try {

        scoped_actor sys{as_actor()->home_system()};
        auto playhead_actor = qml::actorFromQString(as_actor()->home_system(), playheadAddress);

        if (playhead_actor) {
            viewport_renderer_->set_playhead(playhead_actor);

            if (viewport_renderer_->colour_pipeline()) {
                // get the current on screen media source
                auto media_source = utility::request_receive<utility::UuidActor>(
                    *sys, playhead_actor, playhead::media_source_atom_v, true);

                // update the colour pipeline with the media source so it can
                // run its logic to update the view/display attributes etc.
                utility::request_receive<bool>(
                    *sys,
                    viewport_renderer_->colour_pipeline(),
                    playhead::media_source_atom_v,
                    media_source);
            }
        }


    } catch (std::exception &e) {
        spdlog::warn("{} {} ", __PRETTY_FUNCTION__, e.what());
    }
}

void OffscreenViewport::exportToEXR(const media_reader::ImageBufPtr &buf, const caf::uri path) {
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
    outFile.setFrameBuffer((Imf::Rgba *)buf->buffer(), 1, dim.x);
    outFile.writePixels(dim.y);
}

void OffscreenViewport::exportToCompressedFormat(
    const media_reader::ImageBufPtr &buf, const caf::uri path, const std::string &ext) {

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

    QApplication::clipboard()->setImage(im, QClipboard::Clipboard);

    /*int compLevel =
        ext == "TIF" || ext == "TIFF" ? std::max(compression, 1) : (10 - compression) * 10;*/
    // TODO : check m_filePath for extension, if not, add to it. Do it on QML side after merging
    // with new UI branch

    if (path.empty())
        return;

    QImageWriter writer(xstudio::utility::uri_to_posix_path(path).c_str());
    // writer.setCompression(compLevel);
    if (!writer.write(im)) {
        throw std::runtime_error(writer.errorString().toStdString().c_str());
    }
}

void OffscreenViewport::setupTextureAndFrameBuffer(const int width, const int height) {
    if (tex_width_ == width && tex_height_ == height) {
        // bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fboId_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId_, 0);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texId_, 0);
        return;
    }

    if (texId_) {
        glDeleteTextures(1, &texId_);
        glDeleteFramebuffers(1, &fboId_);
        glDeleteTextures(1, &depth_texId_);
    }

    tex_width_  = width;
    tex_height_ = height;

    // create texture
    glGenTextures(1, &texId_);
    glBindTexture(GL_TEXTURE_2D, texId_);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA16F,
        tex_width_,
        tex_height_,
        0,
        GL_RGBA,
        GL_HALF_FLOAT,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    {

        glGenTextures(1, &depth_texId_);
        glBindTexture(GL_TEXTURE_2D, depth_texId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        // NULL means reserve texture memory, but texels are undefined
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT24,
            tex_width_,
            tex_height_,
            0,
            GL_DEPTH_COMPONENT,
            GL_UNSIGNED_BYTE,
            nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // init framebuffer
    glGenFramebuffers(1, &fboId_);
    // bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fboId_);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId_, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texId_, 0);
}

void OffscreenViewport::renderToRGBAHalf16ImageBuffer(
    const int w, const int h,
    media_reader::ImageBufPtr &image,
    const bool syn_fetch_playhead_image) {
    // ensure our GLContext is current
    gl_context_->makeCurrent(surface_);
    if (!gl_context_->isValid()) {
        throw std::runtime_error(
            "OffscreenViewport::renderToImageBuffer - GL Context is not valid.");
    }

    setupTextureAndFrameBuffer(w, h);

    // intialises shaders and textures where necessary
    viewport_renderer_->init();

    // Clearup before render, probably useless for a new buffer
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, w, h);

    // This essential call tells the viewport renderer how to project the
    // viewport area into the glViewport window.
    viewport_renderer_->set_scene_coordinates(
        Imath::V2f(0.0f, 0.0),
        Imath::V2f(w, 0.0),
        Imath::V2f(w, h),
        Imath::V2f(0.0f, h),
        Imath::V2i(w, h));

    if (syn_fetch_playhead_image) {
        media_reader::ImageBufPtr image = viewport_renderer_->get_onscreen_image(true);
        viewport_renderer_->render(image);
    } else {
        viewport_renderer_->render();        
    }

    // Not sure if this is necessary
    glFinish();

    // init RGBA float array
    image->allocate(w * h * 4 * sizeof(half));
    image->set_image_dimensions(Imath::V2i(w, h));
    image.when_to_display_ = utility::clock::now();

    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_ROW_LENGTH, w);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    // read GL pixels to array
    glReadPixels(0, 0, w, h, GL_RGBA, GL_HALF_FLOAT, image->buffer());
    glFinish();
    
    // unbind and delete
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void OffscreenViewport::receive_change_notification(
    ui::viewport::Viewport::ChangeCallbackId id) {

    if (id == ui::viewport::Viewport::ChangeCallbackId::Redraw) {
        if (video_output_actor_) {

            std::vector<media_reader::ImageBufPtr> output_buffers_;
            media_reader::ImageBufPtr ready_buf;
            for (auto &buf : output_buffers_) {
                if (buf.use_count() == 1) {
                    ready_buf = buf;
                    break;
                }
            }
            if (!ready_buf) {
                ready_buf.reset(new media_reader::ImageBuffer());
                output_buffers_.push_back(ready_buf);
            }

            renderToRGBAHalf16ImageBuffer(vid_out_width_, vid_out_height_, ready_buf);

            anon_send(video_output_actor_, ready_buf);
        }
    }
}

void OffscreenViewport::make_conversion_lut() {

    if (half_to_int_32_lut_.empty()) {
        const double int_max = double(std::numeric_limits<uint32_t>::max());
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
OffscreenViewport::rgb96thumbFromHalfFloatImage(const media_reader::ImageBufPtr &image) {

    const Imath::V2i image_size = image->image_size_in_pixels();

    // since we only run this routine ourselves and set-up the image properly
    // this mismatch can't happen but check anyway just in case. Due to padding
    // image buffers are usually a bit larger than the tight pixel size.
    size_t expected_size = image_size.x * image_size.y * sizeof(half) * 4;
    if (expected_size > image->size()) {

        std::string err(fmt::format(
            "{} Image buffer size of {} does not agree with image pixels size of {} ({}x{}).",
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

thumbnail::ThumbnailBufferPtr OffscreenViewport::renderToThumbnail(
    const thumbnail::THUMBNAIL_FORMAT format,
    const int width,
    const bool auto_scale,
    const bool show_annotations) {

    media_reader::ImageBufPtr image = viewport_renderer_->get_onscreen_image(true);

    //std::cerr << "Rendering image " << image->params() << "\n";

    if (!image) {
        std::string err(fmt::format(
            "{} Failed to pull images to offscreen renderer.", __PRETTY_FUNCTION__));
        throw std::runtime_error(err.c_str());
    }

    const Imath::V2i image_dims = image->image_size_in_pixels();
    if (image_dims.x <= 0 || image_dims.y <= 0) {
        std::string err(fmt::format("{} Null image in viewport.", __PRETTY_FUNCTION__));
        throw std::runtime_error(err.c_str());
    }

    float effective_image_height = float(image_dims.y) / image->pixel_aspect();

    if (width <= 0 || auto_scale) {
        viewport_renderer_->set_fit_mode(viewport::FitMode::One2One);
        return renderToThumbnail(format, image_dims.x, int(round(effective_image_height)));
    } else {
        viewport_renderer_->set_fit_mode(viewport::FitMode::Best);
        return renderToThumbnail(
            format, width, int(round(width * effective_image_height / image_dims.x)));
    }
}

thumbnail::ThumbnailBufferPtr OffscreenViewport::renderToThumbnail(
    const thumbnail::THUMBNAIL_FORMAT format, const int width, const int height) {
    media_reader::ImageBufPtr image(new media_reader::ImageBuffer());
    renderToRGBAHalf16ImageBuffer(width, height, image, true);
    thumbnail::ThumbnailBufferPtr r = rgb96thumbFromHalfFloatImage(image);
    r->convert_to(format);
    return r;
}


thumbnail::ThumbnailBufferPtr OffscreenViewport::renderMediaFrameToThumbnail(
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
    viewport_renderer_->set_playhead(local_playhead_);

    scoped_actor sys{as_actor()->home_system()};

    // now set the media source on the local playhead
    utility::request_receive<bool>(
        *sys, local_playhead_, playhead::source_atom_v, std::vector<caf::actor>({media_actor}));

    // now move the playhead to requested frame
    utility::request_receive<bool>(*sys, local_playhead_, playhead::jump_atom_v, media_frame);

    return renderToThumbnail(format, width, auto_scale, show_annotations);
}

thumbnail::ThumbnailBufferPtr OffscreenViewport::renderMediaFrameToThumbnail(
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
    viewport_renderer_->set_playhead(local_playhead_);

    scoped_actor sys{as_actor()->home_system()};

    // now set the media source on the local playhead
    utility::request_receive<bool>(
        *sys, local_playhead_, playhead::source_atom_v, std::vector<caf::actor>({media_actor}));

    // now move the playhead to requested frame
    utility::request_receive<bool>(*sys, local_playhead_, playhead::jump_atom_v, playhead_position_flicks);

    return renderToThumbnail(format, width, auto_scale, show_annotations);
}
