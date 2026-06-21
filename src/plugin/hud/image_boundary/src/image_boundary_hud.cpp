// SPDX-License-Identifier: Apache-2.0
#include "image_boundary_hud.hpp"
#include "image_boundary_hud_renderer.hpp"

#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/media_reader/image_buffer.hpp"
#include "xstudio/global_store/global_store.hpp"
#include "xstudio/utility/blind_data.hpp"
#include "xstudio/ui/viewport/viewport_helpers.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;

ImageBoundaryHUD::ImageBoundaryHUD(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : plugin::HUDPluginBase(cfg, "Image Boundary", init_settings) {

    colour_ =
        add_colour_attribute("Line Colour", "Colour", utility::ColourTriplet(1.0f, 0.0f, 0.0f));
    colour_->set_preference_path("/plugin/image_boundary/colour");
    add_hud_settings_attribute(colour_);

    width_ = add_integer_attribute("Line Width", "Width", 1, 1, 5);
    width_->set_preference_path("/plugin/image_boundary/width");
    add_hud_settings_attribute(width_);
}

plugin::ViewportOverlayRendererPtr
ImageBoundaryHUD::make_overlay_renderer(const std::string &viewport_name) {
    return plugin::ViewportOverlayRendererPtr(new ImageBoundaryRenderer());
}

ImageBoundaryHUD::~ImageBoundaryHUD() = default;

utility::BlindDataObjectPtr ImageBoundaryHUD::onscreen_render_data(
    const media_reader::ImageBufPtr &image,
    const std::string & /*viewport_name*/,
    const utility::Uuid &playhead_uuid,
    const bool is_hero_image,
    const bool images_are_in_grid_layout) const {

    auto r = utility::BlindDataObjectPtr();

    try {
        if (image && visible()) {
            utility::JsonStore j;
            j["colour"] = colour_->value();
            j["width"]  = width_->value();
            r.reset(new HudData(j));
        }

    } catch (std::exception &e) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
    }

    return r;
}

void ImageBoundaryHUD::attribute_changed(
    const utility::Uuid &attribute_uuid, const int /*role*/
) {

    redraw_viewport();
}

XSTUDIO_PLUGIN_DECLARE_BEGIN()

XSTUDIO_REGISTER_PLUGIN(
    ImageBoundaryHUD,
    ImageBoundaryHUD::PLUGIN_UUID,
    Image Boundary HUD,
    plugin_manager::PluginFlags::PF_HEAD_UP_DISPLAY | plugin_manager::PluginFlags::PF_VIEWPORT_OVERLAY,
    false,
    Clement Jovet,
    Image Boundary HUD Plugin,
    1.0.0)

XSTUDIO_PLUGIN_DECLARE_END()
