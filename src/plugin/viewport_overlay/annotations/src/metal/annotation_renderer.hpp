// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/plugin_manager/plugin_base.hpp"
#include "annotation_render_data.hpp"
#include "pixel_patch.hpp"

namespace xstudio::ui::viewport {

class CaptionHandleRenderer {
  public:
    ~CaptionHandleRenderer() = default;

    void render_caption_handle(
        const HandleHoverState &handle_state,
        const Imath::Box2f &caption_box,
        const bool is_live_caption,
        const Imath::V2f *cursor,
        const bool cursor_blink_state,
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float viewport_du_dx,
        const float device_pixel_ratio) {}

  private:
};

class AnnotationsRenderer : public plugin::ViewportOverlayRenderer {

  public:
    AnnotationsRenderer(
        const std::string viewport_name,
        std::atomic_bool &cursor_blink,
        std::atomic_bool &hide_all,
        std::atomic_int *hide_strokes,
        std::atomic_bool *hide_all2) : viewport_name_(std::move(viewport_name)),
      cursor_blink_(cursor_blink),
      hide_all_(hide_all),
      hide_strokes_(hide_strokes),
      hide_per_viewport_(hide_all2) {}

    void render_image_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float viewport_du_dpixel,
        const float device_pixel_ratio,
        const xstudio::media_reader::ImageBufPtr &frame) override {}

    void render_viewport_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_normalised_coords,
        const media_reader::ImageBufDisplaySetPtr &on_screen_frames,
        const float viewport_du_dpixel,
        const float device_pixel_ratio) override {}

    float stack_order() const override { return 2.0f; }

  private:

    std::unique_ptr<CaptionHandleRenderer> texthandle_renderer_;

    const std::string viewport_name_;
    std::atomic_bool &cursor_blink_;
    std::atomic_bool &hide_all_;
    std::atomic_int *hide_strokes_;
    std::atomic_bool *hide_per_viewport_;
};

class AnnotationsExtrasRenderer : public plugin::ViewportOverlayRenderer {

  public:
    AnnotationsExtrasRenderer(PixelPatch &pixel_patch, const std::string &viewport_name)
        : pixel_patch_(pixel_patch), viewport_name_(viewport_name) {}

    void render_image_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float viewport_du_dpixel,
        const float device_pixel_ratio,
        const xstudio::media_reader::ImageBufPtr &frame) override {}

    void render_viewport_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_normalised_coords,
        const media_reader::ImageBufDisplaySetPtr &on_screen_frames,
        const float viewport_du_dpixel,
        const float device_pixel_ratio) override {}

    float stack_order() const override { return 3.0f; }

  private:
    // PixelPatch is (sort-of) thread safe - we need to lock it when using
    PixelPatch &pixel_patch_;

    const std::string viewport_name_;
};

} // namespace xstudio::ui::viewport
