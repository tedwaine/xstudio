// SPDX-License-Identifier: Apache-2.0
#ifdef __apple__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <filesystem>
#include <caf/actor_registry.hpp>


#include "xstudio/ui/qt/opengl/offscreen_viewport.hpp"
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

namespace {

class DefaultFrameGrabber : public ViewportFramePostProcessor {

  public:
    DefaultFrameGrabber() = default;
    ~DefaultFrameGrabber();

    void viewport_capture_framebuffer(
        uint32_t tex_id,
        uint32_t fbo_id,
        const int fb_width,
        const int fb_height,
        const ImageFormat format,
        media_reader::ImageBufPtr &destination_image) override;

    GLuint pixel_buffer_object_ = 0;
    int pix_buf_size_           = 0;
};

// Simple class to split large memcopy across a pool of threads.
//
// More testing needed to check when this actually benefits us and on what
// platforms, but for copying from memory mapped texture buffers into CPU
// RAM it is required for high frame-rate offscreen rendering (e.g. 4k 60Hz
// display on SDI card)
//
class ThreadedMemCopy {
  public:
    ThreadedMemCopy() {
        for (int i = 0; i < num_threads_; ++i) {
            threads_.emplace_back(std::thread(&ThreadedMemCopy::run, this));
        }
    }

    ~ThreadedMemCopy() {

        for ([[maybe_unused]] const auto &t : threads_) {
            // when any thread picks up an em
            {
                std::lock_guard lk(m);
                queue.emplace_back(nullptr, nullptr, 0);
            }
            cv.notify_one();
        }

        for (auto &t : threads_) {
            t.join();
        }
    }

    std::vector<std::thread> threads_;

    struct Job {
        Job(void *d, void *s, size_t _n) : dst(d), src(s), n(_n) {}
        Job(const Job &o) = default;
        void *dst;
        void *src;
        size_t n;

        void do_job() { memcpy(dst, src, n); }
    };

    Job get_job() {
        std::unique_lock lk(m);
        if (queue.empty()) {
            cv.wait(lk, [=] { return !queue.empty(); });
        }
        auto rt = queue.front();
        queue.pop_front();
        if (rt.dst)
            in_progress++;
        return rt;
    }

    void do_memcpy(void *_dst, void *_src, size_t n) {

        size_t step = (((n / num_threads_) / 4096) + 1) * 4096;

        uint8_t *dst = (uint8_t *)_dst;
        uint8_t *src = (uint8_t *)_src;

        while (true) {
            {
                std::lock_guard lk(m);
                queue.emplace_back(dst, src, std::min(n, step));
            }
            cv.notify_one();
            dst += step;
            src += step;
            if (n < step)
                break;
            n -= step;
        }

        std::unique_lock lk(m);
        if (!queue.empty() || in_progress) {
            cv2.wait(lk, [=] { return queue.empty() && !in_progress; });
        }
    }

    void run() {
        while (true) {

            // this blocks until there is something in queue for us
            Job j = get_job();
            if (!j.dst)
                break; // exit
            j.do_job();
            m.lock();
            in_progress--;
            m.unlock();
            cv2.notify_one();
        }
    }

    std::mutex m;
    std::condition_variable cv, cv2;
    std::deque<Job> queue;
    int in_progress = 0;
    const int num_threads_{8};
};

static ThreadedMemCopy threaded_memcopy;
static std::mutex threaded_memcopy_m;

static void threaded_memcpy(void *_dst, void *_src, size_t n) {

    std::unique_lock lk(threaded_memcopy_m);
    threaded_memcopy.do_memcpy(_dst, _src, n);
}

static std::map<ImageFormat, GLint> format_to_gl_tex_format = {
    {ImageFormat::RGBA_8, GL_RGBA8},
    {ImageFormat::RGBA_10_10_10_2, GL_RGBA8},
    {ImageFormat::RGBA_16, GL_RGBA16},
    {ImageFormat::RGBA_16F, GL_RGBA16F},
    {ImageFormat::RGBA_32F, GL_RGBA32F}};

static std::map<ImageFormat, GLint> format_to_gl_pixe_type = {
    {ImageFormat::RGBA_8, GL_UNSIGNED_BYTE},
    {ImageFormat::RGBA_10_10_10_2, GL_UNSIGNED_BYTE},
    {ImageFormat::RGBA_16, GL_UNSIGNED_SHORT},
    {ImageFormat::RGBA_16F, GL_HALF_FLOAT},
    {ImageFormat::RGBA_32F, GL_FLOAT}};

static std::map<ImageFormat, GLint> format_to_bytes_per_pixel = {
    {ImageFormat::RGBA_8, 4},
    {ImageFormat::RGBA_10_10_10_2, 4},
    {ImageFormat::RGBA_16, 8},
    {ImageFormat::RGBA_16F, 8},
    {ImageFormat::RGBA_32F, 16}};

} // namespace

OffscreenViewport::OffscreenViewport(const std::string name, bool sync_with_main_viewports)
    : super(name, sync_with_main_viewports) {

    initGL();

}

void OffscreenViewport::__cleanup() {

    // cleanup is called by our thread on completion, so we can delete
    // ouselves whilst still in the Thread. Qt doesn't let us kill object
    // living in one thread from another thread.
    // gl context must be current for cleanup
    gl_context_->makeCurrent(surface_);
    if (render_control_)
        render_control_->invalidate();
    delete xstudio_viewport_;
    if (post_draw_hook_) {
        post_draw_hook_->cleanup();
    }

    if (texId_) {
        glDeleteTextures(1, &texId_);
        glDeleteFramebuffers(1, &fboId_);
        glDeleteTextures(1, &depth_texId_);
    }

    // teardown the QML gubbins
    delete render_control_;
    delete root_qml_overlays_item_;
    delete qml_component_;
    delete helper_;
    delete quick_win_;
    delete qml_engine_;
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
            throw std::runtime_error("OffscreeninitGL - could not create QOpenGLContext.");
        if (!gl_context_->create()) {
            throw std::runtime_error(
                "OffscreeninitGL - failed to creat GL Context "
                "for offscreen rendering.");
        }

        // This offscreen viewport runs in its own thread
        thread_ = new QThread();

        // we also require a QSurface to use the GL context
        surface_ = new QOffscreenSurface(nullptr, nullptr);
        surface_->setFormat(format);
        surface_->create();

        // Here we set-up the gubbins necessary for rendering QML graphics
        // into the viewport
        render_control_ = new QQuickRenderControl();
        quick_win_      = new QQuickWindow(render_control_);
        qml_engine_     = new QQmlEngine;
        if (!qml_engine_->incubationController())
            qml_engine_->setIncubationController(quick_win_->incubationController());
        qml_engine_->addImportPath("qrc:///");
        qml_engine_->addImportPath("qrc:///extern");

        connect(render_control_, SIGNAL(sceneChanged()), this, SLOT(sceneChanged()));
        connect(render_control_, SIGNAL(renderRequested()), this, SLOT(sceneChanged()));

        // gui plugins..
        qml_engine_->addImportPath(QStringFromStd(utility::xstudio_plugin_dir("/qml")));
        qml_engine_->addPluginPath(QStringFromStd(utility::xstudio_plugin_dir("")));

        gl_context_->moveToThread(thread_);
        qml_engine_->moveToThread(thread_);
        qml_engine_->rootContext()->moveToThread(thread_);
        render_control_->moveToThread(thread_);
        moveToThread(thread_);
        render_control_->prepareThread(thread_);

        helper_ = new qml::Helpers(qml_engine_);
        helper_->moveToThread(thread_);
        qml_engine_->rootContext()->setContextProperty("helpers", helper_);

        const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        qml_engine_->rootContext()->setContextProperty(
            "systemFixedWidthFontFamily", fixedFont.family());

        connect(
            quick_win_,
            &QQuickWindow::beforeRenderPassRecording,
            this,
            &OffscreenViewport::renderViewportUnderQML,
            Qt::DirectConnection);

        quick_win_->setColor(QColor(0, 1, 0, 0));

        thread_->start();

        // Note - the only way I seem to be able to 'cleanly' exit is
        // delete ourselves when the thread quits. Not 100% sure if this
        // is correct approach. I'm still cratching my head as to how
        // to destroy thread_ ... calling deleteLater() directly or
        // using finished signal has no effect.

        connect(thread_, &QThread::finished, thread_, &QThread::deleteLater);
        connect(thread_, &QThread::finished, this, &OffscreenViewport::cleanup);

        // this has no effect!
        // connect(thread_, SIGNAL(finished()), this, SLOT(deleteLater()));
    }
}

void OffscreenViewport::__stop() {
    thread_->quit();
    thread_->wait();
    delete thread_;
}

void OffscreenViewport::__renderViewportUnderQML() {

    quick_win_->beginExternalCommands();

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    xstudio_viewport_->init();

    if (image_to_render_) {
        xstudio_viewport_->render(image_to_render_);
    } else {
        xstudio_viewport_->render();
    }

    glPopClientAttrib();

    quick_win_->endExternalCommands();
}

bool OffscreenViewport::setupTextureAndFrameBuffer(
    const int width, const int height, const ImageFormat format) {

    if (tex_width_ == width && tex_height_ == height && format == vid_out_format_) {
        // bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fboId_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId_, 0);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texId_, 0);
        return false;
    }

    GLuint old_tex_id       = texId_;
    GLuint old_fbo_id       = fboId_;
    GLuint old_depth_tex_id = depth_texId_;

    tex_width_      = width;
    tex_height_     = height;
    vid_out_format_ = format;

    // create texture
    glGenTextures(1, &texId_);
    glBindTexture(GL_TEXTURE_2D, texId_);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format_to_gl_tex_format[vid_out_format_],
        tex_width_,
        tex_height_,
        0,
        GL_RGBA,
        GL_UNSIGNED_SHORT,
        nullptr);

    GLint iTexFormat;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &iTexFormat);
    if (iTexFormat != format_to_gl_tex_format[vid_out_format_]) {
        spdlog::warn(
            "{} offscreen viewport texture internal format is {:#x}, which does not match "
            "desired format {:#x}",
            __PRETTY_FUNCTION__,
            iTexFormat,
            format_to_gl_tex_format[vid_out_format_]);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    // init framebuffer
    glGenFramebuffers(1, &fboId_);
    // bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fboId_);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId_, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texId_, 0);

    if (old_tex_id) {
        // clean-up the old textures & fbo. Note that doing this before generating
        // a new texture fails when the texture format changes - we get a black
        // image.
        glDeleteTextures(1, &old_tex_id);
        glDeleteFramebuffers(1, &old_fbo_id);
        glDeleteTextures(1, &old_depth_tex_id);
    }

    return true;
}

void OffscreenViewport::render(
    const int w,
    const int h,
    const viewport::ImageFormat format,
    const bool sync_fetch_playhead_image,
    const utility::time_point &tp,
    const media_reader::ImageBufPtr &image_to_use,
    const bool include_overlays,
    const bool include_drawings) {

    // ensure our GLContext is current
    if (!gl_context_->makeCurrent(surface_) || !gl_context_->isValid()) {
        throw std::runtime_error("OffscreenrenderToImageBuffer - GL Context is not valid.");
    }

    // glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
    //                      GL_DEBUG_SEVERITY_NOTIFICATION, -1, "OffscreenViewport::render
    //                      START");

    // No QML .. much simpler. Just set-up and render our xstudio viewport
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    // intialises shaders and textures where necessary
    xstudio_viewport_->init();

    // create the FBO and texture for us to render into
    const bool updateTarget = setupTextureAndFrameBuffer(w, h, format);

    // auto t1 = utility::clock::now();

    glPopClientAttrib();

    // This essential call tells the viewport renderer how to project the
    // viewport area into the glViewport window.
    xstudio_viewport_->set_geometry(
        0.0f, // x offset
        0.0f, // y offset
        w,    // viewport width in window
        h,    // viewport height in window
        w,    // window width
        h,    // window height,
        1.0f  // pixel scaling (high DPI support)
    );

    if (include_overlays && loadQMLOverlays()) {

        // If we are rendering a supplied image, store it for when we do
        // the render, or we call 'prepare_render_data' which pre-fetches the image
        // buffer from the playhead attached to the viewport and stores. This
        // is all handled by the Viewport instance
        if (image_to_use) {
            image_to_render_ = image_to_use;
        } else {
            image_to_render_ = media_reader::ImageBufPtr();
            if (sync_fetch_playhead_image) {

                xstudio_viewport_->prepare_render_data(utility::time_point(), true);

            } else if (tp != utility::time_point()) {
                xstudio_viewport_->prepare_render_data(tp);
            } else {
                xstudio_viewport_->prepare_render_data();
            }
        }

        glActiveTexture(GL_TEXTURE0);

        // now do some set-up for QML engine
        if (updateTarget) {
            quick_win_->setRenderTarget(
                QQuickRenderTarget::fromOpenGLTexture(texId_, QSize(w, h)));
        }

        root_qml_overlays_item_->setWidth(w);
        root_qml_overlays_item_->setHeight(h);

        // convert the image boundary in the viewport into plain pixels
        const std::vector<Imath::Box2f> image_boxes =
            xstudio_viewport_->image_bounds_in_viewport_pixels();
        QVariantList v;
        for (const auto &box : image_boxes) {
            QRectF imageBoundsInViewportPixels(
                box.min.x, box.min.y, box.max.x - box.min.x, box.max.y - box.min.y);
            v.append(imageBoundsInViewportPixels);
        }

        // these properties on XsOffscreenViewportOverlays mirror the same
        // properties provided by XsViewport - some overlay/HUD QML items access
        // these properties so they know how to compute their geometrty in
        // the QML coordinates to overlay the xSTUDIO image.
        root_qml_overlays_item_->setProperty("imageBoundariesInViewport", v);

        root_qml_overlays_item_->setProperty(
            "playheadUuid", QUuidFromUuid(xstudio_viewport_->playhead_uuid()));

        if (sync_fetch_playhead_image) {
            sync_python_hud_data();
        }

        const std::vector<Imath::V2i> resolutions = xstudio_viewport_->image_resolutions();
        QVariantList rs;
        for (const auto &r : resolutions) {
            rs.append(QSize(r.x, r.y));
        }
        root_qml_overlays_item_->setProperty("imageResolutions", rs);

        root_qml_overlays_item_->setProperty("sessionActorAddr", session_actor_addr_);
        quick_win_->setWidth(w);
        quick_win_->setHeight(h);
        quick_win_->setGeometry(0, 0, w, h);

        // auto t2 = utility::clock::now();

        render_control_->polishItems();
        render_control_->beginFrame();
        render_control_->sync();

        // note we have a signal/slot connection that causes renderViewportUnderQML
        // to be called at the right moment so the xstudio Viewport can be drawn before
        // the QML is rendered
        render_control_->render();
        render_control_->endFrame();

    } else {

        // Clearup before render, probably useless for a new buffer
        glViewport(0, 0, w, h);

        if (image_to_use) {
            xstudio_viewport_->render(image_to_use, include_drawings);
        } else {
            if (sync_fetch_playhead_image) {
                xstudio_viewport_->prepare_render_data(utility::clock::now(), true);
            } else if (tp != utility::time_point()) {
                xstudio_viewport_->prepare_render_data(tp);
            } else {
                xstudio_viewport_->prepare_render_data();
            }
            xstudio_viewport_->render();
        }

        // auto t2 = utility::clock::now();

        glActiveTexture(GL_TEXTURE0);
    }

    glFlush();

    if (post_draw_hook_) {
        post_draw_hook_->viewport_post_process_framebuffer(
            texId_,
            fboId_,
            w,
            h,
            format,
            xstudio_viewport_->on_screen_frames(),
            xstudio_viewport_->projection_matrix());
    }

    // auto t3 = utility::clock::now();

    // Not sure if this is necessary
    // glFinish();

    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
    //                      GL_DEBUG_SEVERITY_NOTIFICATION, -1, "OffscreenViewport::render
    //                      END");
}

QQuickGraphicsDevice OffscreenViewport::graphics_device() {

    return QQuickGraphicsDevice::fromOpenGLContext(gl_context_);

}

void OffscreenViewport::capture_framebuffer(
    const int w,
    const int h,
    const viewport::ImageFormat format,
    media_reader::ImageBufPtr &destination_image) 
{

    if (!post_draw_hook_)
        post_draw_hook_.reset(new DefaultFrameGrabber());
    
    post_draw_hook_->viewport_capture_framebuffer(
        texId_, fboId_, w, h, format, destination_image);

}

DefaultFrameGrabber::~DefaultFrameGrabber() { glDeleteBuffers(1, &pixel_buffer_object_); }

void DefaultFrameGrabber::viewport_capture_framebuffer(
    uint32_t tex_id,
    uint32_t fbo_id,
    const int fb_width,
    const int fb_height,
    const ImageFormat format,
    media_reader::ImageBufPtr &destination_image) {

    size_t pix_buf_size = fb_width * fb_height * format_to_bytes_per_pixel[format];

    // init RGBA float array
    destination_image = get_video_output_frame();
    destination_image->allocate(pix_buf_size);
    destination_image->set_image_dimensions(Imath::V2i(fb_width, fb_height));
    destination_image.when_to_display()         = utility::clock::now();
    destination_image->params()["pixel_format"] = (int)format;

    if (!pixel_buffer_object_) {
        glGenBuffers(1, &pixel_buffer_object_);
    }

    if (static_cast<int>(pix_buf_size) != pix_buf_size_) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer_object_);
        glBufferData(GL_PIXEL_PACK_BUFFER, pix_buf_size, nullptr, GL_STREAM_COPY);
        pix_buf_size_ = pix_buf_size;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    int skip_rows, skip_pixels, row_length, alignment;
    glGetIntegerv(GL_PACK_SKIP_ROWS, &skip_rows);
    glGetIntegerv(GL_PACK_SKIP_PIXELS, &skip_pixels);
    glGetIntegerv(GL_PACK_ROW_LENGTH, &row_length);
    glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);

    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_ROW_LENGTH, fb_width);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, format_to_gl_pixe_type[format], nullptr);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer_object_);
    void *mappedBuffer = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

    // auto t4 = utility::clock::now();

    threaded_memcpy(destination_image->buffer(), mappedBuffer, pix_buf_size);

    // now mapped buffer contains the pixel data
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    // auto t5 = utility::clock::now();

    // auto tt = utility::clock::now();

    // TODO: Gather stats on draw times etc and send to video_output_actor_
    // so it can monitor performance

    /*std::cerr << "Draw time "  <<
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << "\n"; std::cerr <<
    "Overlays time "  << std::chrono::duration_cast<std::chrono::milliseconds>(t3-t2).count() <<
    "\n"; std::cerr << "Map buffer time "  <<
    std::chrono::duration_cast<std::chrono::milliseconds>(t4-t3).count() << "\n"; std::cerr <<
    "Copy buffer time "  << std::chrono::duration_cast<std::chrono::milliseconds>(t5-t4).count()
    << "\n";*/

    glBindTexture(GL_TEXTURE_2D, 0);

    glPixelStorei(GL_PACK_SKIP_ROWS, skip_rows);
    glPixelStorei(GL_PACK_SKIP_PIXELS, skip_pixels);
    glPixelStorei(GL_PACK_ROW_LENGTH, row_length);
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
}
