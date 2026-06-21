// SPDX-License-Identifier: Apache-2.0
#include <filesystem>
#include <caf/actor_registry.hpp>

#include "xstudio/ui/qt/metal/offscreen_viewport.hpp"
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
        media_reader::ImageBufPtr &destination_image);

};

} // namespace

OffscreenViewport::OffscreenViewport(const std::string name, bool sync_with_main_viewports)
    : super(name, sync_with_main_viewports) {

}

void OffscreenViewport::__cleanup() {

    video_output_actor_ = caf::actor();
}

void OffscreenViewport::__stop() {
    thread_->quit();
    thread_->wait();
    delete thread_;
}

void OffscreenViewport::__renderViewportUnderQML() {

    quick_win_->beginExternalCommands();

    xstudio_viewport_->init();

    if (image_to_render_) {
        xstudio_viewport_->render(image_to_render_);
    } else {
        xstudio_viewport_->render();
    }

    quick_win_->endExternalCommands();
}

QQuickGraphicsDevice OffscreenViewport::graphics_device() {

    return QQuickGraphicsDevice();//::fromOpenGLContext(gl_context_);

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

    if (!post_draw_hook_)
        post_draw_hook_.reset(new DefaultFrameGrabber());
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
        0, 0, w, h, format, destination_image);

}

DefaultFrameGrabber::~DefaultFrameGrabber() { }

void DefaultFrameGrabber::viewport_capture_framebuffer(
    uint32_t tex_id,
    uint32_t fbo_id,
    const int fb_width,
    const int fb_height,
    const ImageFormat format,
    media_reader::ImageBufPtr &destination_image) {

    size_t pix_buf_size = fb_width * fb_height;//* format_to_bytes_per_pixel[format];

    // init RGBA float array
    destination_image = get_video_output_frame();
    destination_image->allocate(pix_buf_size);
    destination_image->set_image_dimensions(Imath::V2i(fb_width, fb_height));
    destination_image.when_to_display()         = utility::clock::now();
    destination_image->params()["pixel_format"] = (int)format;

    
}
