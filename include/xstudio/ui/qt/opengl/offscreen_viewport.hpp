// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/ui/qt/offscreen_viewport_base.hpp"
#include "xstudio/ui/qt/opengl/viewport_widget.hpp"
#include "xstudio/thumbnail/thumbnail.hpp"
#include "xstudio/ui/viewport/viewport_gpu_post_processor.hpp"

#include <QString>
#include <QUrl>
#include <QObject>
// #include <QOpenGLFramebufferObject>
#include <QImage>

#undef __GLEW_H__
#include <QOpenGLContext>

#include <QOffscreenSurface>

namespace opengl {
class OpenGLViewportRenderer;
}

class QQuickWindow;
class QQuickItem;
class QQmlComponent;
class QQuickRenderControl;
class QQmlEngine;

namespace xstudio::ui {

namespace qml {
    class Helpers;
}

namespace qt {

    class OffscreenViewport : public OffscreenViewportBase {

        Q_OBJECT
        using super = OffscreenViewportBase;

      public:

        OffscreenViewport(const std::string name, bool sync_to_other_viewports);
        ~OffscreenViewport() override = default;

      protected:

        /* To implement the offscreen viewport we must provide 
        implementations for these virtual functions */

        // Called on exit
        void __cleanup() override;

        // Called when we need to render the viewport UNDER qml
        void __renderViewportUnderQML() override;

        void render(
            const int w,
            const int h,
            const viewport::ImageFormat format,
            const bool sync_fetch_playhead_image,
            const utility::time_point &tp,
            const media_reader::ImageBufPtr &image_to_use = media_reader::ImageBufPtr(),
            const bool include_overlays                   = true,
            const bool include_drawings                   = true) override;

        void capture_framebuffer(
            const int w,
            const int h,
            const viewport::ImageFormat format,
            media_reader::ImageBufPtr &destination_image) override;

        void __stop() override;
        
      private:

        bool setupTextureAndFrameBuffer(
            const int width, const int height, const viewport::ImageFormat format);

        void sync_python_hud_data();

        void initGL();

        bool loadQMLOverlays();

        QOpenGLContext *gl_context_               = {nullptr};
        QOffscreenSurface *surface_               = {nullptr};
        QThread *thread_                          = {nullptr};

        int tex_width_      = 0;
        int tex_height_     = 0;
        GLuint texId_       = 0;
        GLuint fboId_       = 0;
        GLuint depth_texId_ = 0;

        QQuickWindow *quick_win_             = nullptr;
        QQuickItem *root_qml_overlays_item_  = nullptr;
        QQmlComponent *qml_component_        = nullptr;
        QQuickRenderControl *render_control_ = nullptr;
        QQmlEngine *qml_engine_              = nullptr;
        ui::qml::Helpers *helper_            = nullptr;
        bool overlays_loaded_                = false;

    };
} // namespace qt
} // namespace xstudio::ui