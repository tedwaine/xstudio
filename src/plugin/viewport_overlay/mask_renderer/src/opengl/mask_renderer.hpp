// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "../mask_renderer_plugin.hpp"
#include "xstudio/ui/opengl/opengl_text_rendering.hpp"

namespace xstudio::ui::viewport {

class MaskRenderer : public plugin::ViewportOverlayRenderer {

  public:
    void render_image_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float viewport_du_dpixel,
        const float device_pixel_ratio,
        const xstudio::media_reader::ImageBufPtr &frame) override;

  private:
    void render_mask(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float viewport_du_dpixel,
        const float device_pixel_ratio,
        const xstudio::media_reader::ImageBufPtr &frame,
        const Mask &mask);

    void init_overlay_opengl();

    std::unique_ptr<xstudio::ui::opengl::GLShaderProgram> shader_;
    GLuint vertex_buffer_object_;
    GLuint vertex_array_object_;
    std::unique_ptr<xstudio::ui::opengl::OpenGLTextRendererSDF> text_renderer_;
    std::vector<float> precomputed_text_vertex_buffer_;
    size_t last_mask_hash_ = 0;
    Imath::Box2i last_bounds_;
    float font_scale_    = 0.0f;
    size_t num_vertices_ = 0;
};

} // namespace xstudio::ui::viewport
