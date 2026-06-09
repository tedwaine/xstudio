// SPDX-License-Identifier: Apache-2.0
#include <caf/all.hpp>

#include <chrono>

#include "wipe_viewport_layout.hpp"
#include "wipe_viewport_layout_renderer.hpp"
#include "xstudio/media_reader/image_buffer.hpp"

using namespace xstudio::utility;
using namespace xstudio::ui::viewport;
using namespace xstudio;

using namespace xstudio::ui::opengl;

WipeViewportLayout::WipeViewportLayout(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : ViewportLayoutPlugin(cfg, init_settings) {

    wipe_position_ = add_vec4f_attribute(
        "Wipe Position", "Wipe Position", Imath::V4f(0.5f, 0.5f, 0.0f, 1.0f));
    wipe_position_->set_redraw_viewport_on_change(true);
    wipe_position_->set_role_data(
        module::Attribute::ToolTip,
        "Spacing between images in grid layout as a % of image size.");
    wipe_position_->expose_in_ui_attrs_group("wipe_layout_attrs");

    add_layout_mode(
        "Wipe", 2.0, playhead::AssemblyMode::AM_TEN, playhead::AutoAlignMode::AAM_ALIGN_FRAMES);

    add_viewport_layout_qml_overlay(
        "Wipe",
        R"(
            import WipeLayoutOverlay 1.0
            WipeLayoutOverlay {
            }
        )");
}

void WipeViewportLayout::do_layout(
    const std::string &layout_mode,
    const media_reader::ImageBufDisplaySetPtr &image_set,
    media_reader::ImageSetLayoutData &layout_data) {
    const int num_images = image_set->num_onscreen_images();
    if (!num_images) {
        return;
    }

    // if 'hero' image is at index 0, we wipe between image 0 and image 1
    // otherwise we wipe between hero image and image 0.
    int wipeA = image_set->hero_sub_playhead_index();

    layout_data.image_draw_order_hint_.push_back(wipeA);
    if (num_images > 1) {
        int wipeB = image_set->previous_hero_sub_playhead_index() != -1
                        ? image_set->previous_hero_sub_playhead_index()
                    : wipeA ? 0
                            : 1;
        layout_data.image_draw_order_hint_.push_back(wipeB);
    }

    // identity matrices here, no transform for A/B wipes
    layout_data.image_transforms_.resize(image_set->num_onscreen_images());
    layout_data.custom_layout_data_["wipe_pos"] = wipe_position_->value().x;

    layout_data.layout_aspect_ = image_layout_aspect(image_set->onscreen_image(wipeA));
    layout_data.draw_hero_overlays_only_ = false;
}

ViewportRenderer *WipeViewportLayout::make_renderer(
    const std::string &window_id, const utility::JsonStore &prefs) {
    return new ViewportWipeRenderer(window_id, prefs);
}

extern "C" {
plugin_manager::PluginFactoryCollection *plugin_factory_collection_ptr() {
    return new plugin_manager::PluginFactoryCollection(
        std::vector<std::shared_ptr<plugin_manager::PluginFactory>>(
            {std::make_shared<plugin_manager::PluginFactoryTemplate<WipeViewportLayout>>(
                WipeViewportLayout::PLUGIN_UUID,
                "WipeViewportLayout",
                plugin_manager::PluginFlags::PF_VIEWPORT_RENDERER,
                false,
                "Ted Waine",
                "Wipe Viewport Layout Plugin")}));
}
}
