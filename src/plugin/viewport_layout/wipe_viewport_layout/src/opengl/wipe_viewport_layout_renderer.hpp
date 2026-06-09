// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"

namespace xstudio::ui::viewport {

class ViewportWipeRenderer : public opengl::OpenGLViewportRenderer {

  public:

    ViewportWipeRenderer(
        const std::string &window_id,
        const utility::JsonStore &prefs
    );

    ~ViewportWipeRenderer() override;

    void pre_init() override;

    void draw_image(
        const media_reader::ImageBufPtr &image,
        const media_reader::ImageSetLayoutDataPtr &layout_data,
        const int index,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_space,
        const float viewport_du_dx) override;

    unsigned int wipe_vbo_ = 0;
    unsigned int wipe_vao_ = 0;
};

} // namespace xstudio::ui::viewport
