// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/ui/opengl/shader_program_base.hpp"
#include "xstudio/plugin_manager/hud_plugin.hpp"

namespace xstudio::ui::viewport {

class ImageBoundaryHUD : public plugin::HUDPluginBase {
  public:
    ImageBoundaryHUD(caf::actor_config &cfg, const utility::JsonStore &init_settings);

    ~ImageBoundaryHUD();

    void attribute_changed(
        const utility::Uuid &attribute_uuid, const int /*role*/
        ) override;

    class HudData : public utility::BlindDataObject {
      public:
        HudData(const utility::JsonStore &j) : hud_params_(j) {}
        ~HudData() override = default;

        const utility::JsonStore hud_params_;
    };

    static inline const utility::Uuid PLUGIN_UUID =
        utility::Uuid("95268f7c-88d1-48da-8543-c5275ef5b2c5");

  protected:
    utility::BlindDataObjectPtr onscreen_render_data(
        const media_reader::ImageBufPtr &,
        const std::string & /*viewport_name*/,
        const utility::Uuid &playhead_uuid,
        const bool is_hero_image,
        const bool images_are_in_grid_layout) const override;

    plugin::ViewportOverlayRendererPtr
    make_overlay_renderer(const std::string &viewport_name) override;

  private:
    module::ColourAttribute *colour_ = nullptr;
    module::IntegerAttribute *width_ = nullptr;
};

} // namespace xstudio::ui::viewport
