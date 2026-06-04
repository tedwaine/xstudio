// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/thumbnail/thumbnail.hpp"
#include "xstudio/ui/qml/actor_object.hpp"
#include "xstudio/ui/viewport/viewport_gpu_post_processor.hpp"
#include "xstudio/ui/viewport/viewport.hpp"

#include <QString>
#include <QUrl>
#include <QObject>
#include <QImage>

#include <QOffscreenSurface>

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

    class OffscreenViewportBase : public caf::mixin::actor_object<QObject> {

        Q_OBJECT
        using super = caf::mixin::actor_object<QObject>;

      public:

        OffscreenViewportBase(const std::string name, bool sync_to_other_viewports);
        ~OffscreenViewportBase() override = default;

        // Direct rendering to an output file
        void
        renderSnapshot(const int width, const int height, const caf::uri path = caf::uri());

        void setPlayhead(const QString &playheadAddress);

        std::string name() { return xstudio_viewport_->name(); }

        void stop() { __stop(); }

      public slots:

        void cleanup() { __cleanup(); }
        void sceneChanged();
        void renderViewportUnderQML() { __renderViewportUnderQML(); }

      protected:

        /* To implement the offscreen viewport we must provide 
        implementations for these virtual functions */

        // Called on exit
        virtual void __cleanup() = 0;

        // Called when we need to render the viewport UNDER qml
        virtual void __renderViewportUnderQML() = 0;

        virtual void render(
            const int w,
            const int h,
            const viewport::ImageFormat format,
            const bool sync_fetch_playhead_image,
            const utility::time_point &tp,
            const media_reader::ImageBufPtr &image_to_use = media_reader::ImageBufPtr(),
            const bool include_overlays                   = true,
            const bool include_drawings                   = true) = 0;

        virtual void capture_framebuffer(
            const int w,
            const int h,
            const viewport::ImageFormat format,
            media_reader::ImageBufPtr &destination_image) = 0;

        virtual void __stop() = 0;

        caf::actor_system &system() { return self()->home_system(); }

        void receive_change_notification(viewport::Viewport::ChangeCallbackId id);

        thumbnail::ThumbnailBufferPtr renderOffscreen(
            const int w,
            const int h,
            const media_reader::ImageBufPtr &image = media_reader::ImageBufPtr());

        media_reader::ImageBufPtr renderToImageBuf(
            int width,
            int height,
            const bool include_image    = true,
            const bool include_overlays = true,
            const bool include_drawings = true);

        thumbnail::ThumbnailBufferPtr renderToThumbnail(
            const thumbnail::THUMBNAIL_FORMAT format,
            const int width,
            const bool auto_scale,
            const bool show_annotations);

        thumbnail::ThumbnailBufferPtr renderToThumbnail(
            const thumbnail::THUMBNAIL_FORMAT format, const int width, const int height);

        void exportToEXR(const media_reader::ImageBufPtr &image, const caf::uri path);

        virtual void renderToImageBuffer(
            const int w,
            const int h,
            media_reader::ImageBufPtr &dest_image,
            const viewport::ImageFormat format,
            const bool force_sync,
            const utility::time_point &tp                 = utility::time_point(),
            const media_reader::ImageBufPtr &image_to_use = media_reader::ImageBufPtr(),
            const bool include_overlays                   = true,
            const bool include_drawings                   = true);

        media_reader::ImageBufPtr renderMediaFrameToImage(
            caf::actor media_actor,
            const int media_frame,
            const int width,
            const int height,
            const bool include_image    = true,
            const bool include_overlays = true,
            const bool include_drawings = true);

        thumbnail::ThumbnailBufferPtr renderMediaFrameToThumbnail(
            caf::actor media_actor,
            const int media_frame,
            const thumbnail::THUMBNAIL_FORMAT format,
            const int width,
            const bool auto_scale,
            const bool show_annotations);

        thumbnail::ThumbnailBufferPtr renderMediaFrameToThumbnail(
            caf::actor media_actor,
            const timebase::flicks media_timepoint,
            const thumbnail::THUMBNAIL_FORMAT format,
            const int width,
            const bool auto_scale,
            const bool show_annotations);

        void exportToCompressedFormat(
            const media_reader::ImageBufPtr &buf,
            const caf::uri path,
            const std::string &ext,
            const bool has_alpha = false);

        void make_conversion_lut();

        thumbnail::ThumbnailBufferPtr
        rgb96thumbFromHalfFloatImage(const media_reader::ImageBufPtr &image);

        ui::viewport::Viewport *xstudio_viewport_ = nullptr;
        viewport::ViewportFramePostProcessorPtr post_draw_hook_;

        // TODO: will remove once everything done
        const char *formatSuffixes[4] = {"EXR", "JPG", "PNG", "TIFF"};

        int vid_out_width_                    = 0;
        int vid_out_height_                   = 0;
        viewport::ImageFormat vid_out_format_ = viewport::ImageFormat::RGBA_16;
        caf::actor video_output_actor_;
        media_reader::ImageBufPtr last_rendered_frame_;
        media_reader::ImageBufPtr image_to_render_;
        std::vector<uint32_t> half_to_int_32_lut_;

        caf::actor local_playhead_;
        QString session_actor_addr_;

    };
} // namespace qt
} // namespace xstudio::ui