#pragma once
#include "xstudio/ui/opengl/shader_program_base.hpp"
#include "xstudio/ui/opengl/opengl_text_rendering.hpp"
#include "../media_metadata_hud.hpp"

/* We defined a separate class to take care of rendering graphics into
the xstudio viewport. Be aware the instance(s) of this class runs in
a separate thread to the main plugin class instance, don't share
data directly. Rather, we use our MediaMetadata class to pass data from
the plugin class to the renderer.
*/
namespace xstudio::ui::viewport {

    class MediaMetadataRenderer : public plugin::ViewportOverlayRenderer {

    public:

        MediaMetadataRenderer() = default;
        ~MediaMetadataRenderer() = default;

        void render_image_overlay(
            const Imath::M44f &transform_window_to_viewport_space,
            const Imath::M44f &transform_viewport_to_image_space,
            const float viewport_du_dpixel,
            const float device_pixel_ratio,
            const xstudio::media_reader::ImageBufPtr &frame) override;

        void init_overlay_opengl();

        std::unique_ptr<xstudio::ui::opengl::GLShaderProgram> shader_;
        GLuint vbo_;
        GLuint vao_;
        std::unique_ptr<xstudio::ui::opengl::OpenGLTextRendererSDF> text_renderer_;
        DisplaySettingsPtr display_settings_;
    };

}