// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio::ui::viewport {

class ImageBoundaryRenderer : public plugin::ViewportOverlayRenderer {

  public:

    void render_image_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float /*viewport_du_dpixel*/,
        const float /*device_pixel_ratio*/,
        const xstudio::media_reader::ImageBufPtr &frame) override;
    void init_overlay_opengl();

    std::unique_ptr<xstudio::ui::opengl::GLShaderProgram> shader_;
    GLuint vertex_buffer_object_;
    GLuint vertex_array_object_;
};
} // namespace xstudio::ui::viewport