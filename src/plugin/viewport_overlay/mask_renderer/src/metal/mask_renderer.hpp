// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "../mask_renderer_plugin.hpp"

namespace xstudio::ui::viewport {

class MaskRenderer : public plugin::ViewportOverlayRenderer {

  public:
    void render_image_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float viewport_du_dpixel,
        const float device_pixel_ratio,
        const xstudio::media_reader::ImageBufPtr &frame) override {}

  private:

};

} // namespace xstudio::ui::viewport
