// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"
#include "xstudio/ui/opengl/opengl_offscreen_renderer.hpp"
#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio::ui::viewport {

class ViewportCompositeRenderer : public opengl::OpenGLViewportRenderer {

  public:
    ViewportCompositeRenderer(
        const std::string &window_id, const utility::JsonStore &prefs);

    ~ViewportCompositeRenderer() override = default;

    void pre_init() override;

    void draw_image(
        const media_reader::ImageBufPtr &image,
        const media_reader::ImageSetLayoutDataPtr &layout_data,
        const int index,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_space,
        const float viewport_du_dx) override;

    void render_difference(
        const media_reader::ImageBufPtr &image_to_be_drawn,
        const bool first_im,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_space,
        const float viewport_du_dx,
        const utility::JsonStore &params);

    opengl::OpenGLOffscreenRendererPtr offscreen_texture_target_A_;
    opengl::OpenGLOffscreenRendererPtr offscreen_texture_target_B_;
    std::unique_ptr<opengl::GLShaderProgram> shader_;
};

} // namespace xstudio::ui::viewport