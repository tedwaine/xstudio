// SPDX-License-Identifier: Apache-2.0
#include <caf/all.hpp>

#include <chrono>

#include "composite_viewport_layout.hpp"
#include "composite_viewport_layout_renderer.hpp"
#include "xstudio/media_reader/image_buffer.hpp"

using namespace xstudio::utility;
using namespace xstudio::ui::viewport;
using namespace xstudio;

CompositeViewportLayout::CompositeViewportLayout(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : ViewportLayoutPlugin(cfg, init_settings) {

    blend_ratio_ = add_float_attribute("Blend Ratio", "Blend Ratio", 0.5f, 0.0f, 1.0f, 0.005f);
    difference_boost_ =
        add_float_attribute("Difference Boost", "Diff Boost", 0.0f, -5.0f, 5.0f, 0.005f);
    monochrome_ = add_boolean_attribute("Monochrome", "Monochrome", true);

    add_layout_mode(
        "Over", 1.0, playhead::AssemblyMode::AM_TEN, playhead::AutoAlignMode::AAM_ALIGN_FRAMES);

    add_layout_mode(
        "A/B Blend",
        2.0,
        playhead::AssemblyMode::AM_TEN,
        playhead::AutoAlignMode::AAM_ALIGN_FRAMES);

    add_layout_mode(
        "Add", 3.0, playhead::AssemblyMode::AM_TEN, playhead::AutoAlignMode::AAM_ALIGN_FRAMES);

    add_layout_mode(
        "A/B Difference",
        4.0,
        playhead::AssemblyMode::AM_TEN,
        playhead::AutoAlignMode::AAM_ALIGN_FRAMES);

    add_layout_mode(
        "Screen",
        5.0,
        playhead::AssemblyMode::AM_TEN,
        playhead::AutoAlignMode::AAM_ALIGN_FRAMES);

    add_layout_settings_attribute(blend_ratio_, "A/B Blend");
    add_layout_settings_attribute(difference_boost_, "A/B Difference");
    add_layout_settings_attribute(monochrome_, "A/B Difference");

    blend_ratio_->set_preference_path("/plugin/composite_vp_modes/blend_ratio");
    difference_boost_->set_preference_path("/plugin/composite_vp_modes/difference_boost");
    monochrome_->set_preference_path("/plugin/composite_vp_modes/difference_monochrome");
}

ViewportRenderer *CompositeViewportLayout::make_renderer(
    const std::string &window_id, const utility::JsonStore &prefs) {
    return new ViewportCompositeRenderer(window_id, prefs);
}

void CompositeViewportLayout::do_layout(
    const std::string &layout_mode,
    const media_reader::ImageBufDisplaySetPtr &image_set,
    media_reader::ImageSetLayoutData &layout_data) {
    const int num_images = image_set->num_onscreen_images();
    if (!num_images) {
        return;
    }

    layout_data.draw_hero_overlays_only_ = true;

    if (num_images >= 2 && (layout_mode == "A/B Blend" || layout_mode == "A/B Difference" ||
                            layout_mode == "Screen")) {

        // if 'hero' image is at index 0, we wipe between image 0 and image 1
        // otherwise we wipe between hero image and image 0.
        int wipeA                                   = image_set->hero_sub_playhead_index();
        layout_data.custom_layout_data_["first_im"] = wipeA;

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

        if (layout_mode == "A/B Blend") {
            layout_data.custom_layout_data_["blend_ratio"] = blend_ratio_->value();
            layout_data.custom_layout_data_["mode"]        = 0;
        } else if (layout_mode == "A/B Difference") {
            layout_data.custom_layout_data_["mode"]       = 3;
            layout_data.custom_layout_data_["monochrome"] = monochrome_->value();
            layout_data.custom_layout_data_["boost"]      = difference_boost_->value();
        } else if (layout_mode == "Screen") {
            layout_data.custom_layout_data_["mode"]   = 4;
            layout_data.custom_layout_data_["screen"] = true;
        }
        layout_data.layout_aspect_ = image_layout_aspect(image_set->onscreen_image(wipeA));
        layout_data.custom_layout_data_["image_aspect"] = 1.0f/layout_data.layout_aspect_;


    } else {

        // draw the images in reverse order. So Image selection 0 is drawn
        // 'over' image 1 etc (if in over mode)
        for (int i = 0; i < num_images; ++i) {
            layout_data.image_draw_order_hint_.push_back(num_images - i - 1);
        }
        layout_data.custom_layout_data_["first_im"] =
            layout_data.image_draw_order_hint_.front();

        // identity matrices here, no transform for 'over' mode'
        layout_data.image_transforms_.resize(image_set->num_onscreen_images());

        layout_data.layout_aspect_ =
            image_layout_aspect(image_set->onscreen_image(num_images - 1));
        layout_data.custom_layout_data_["mode"] = layout_mode == "Over" ? 1 : 2;
    }
}

extern "C" {
plugin_manager::PluginFactoryCollection *plugin_factory_collection_ptr() {
    return new plugin_manager::PluginFactoryCollection(
        std::vector<std::shared_ptr<plugin_manager::PluginFactory>>(
            {std::make_shared<plugin_manager::PluginFactoryTemplate<CompositeViewportLayout>>(
                CompositeViewportLayout::PLUGIN_UUID,
                "CompositeViewportLayout",
                plugin_manager::PluginFlags::PF_VIEWPORT_RENDERER,
                false,
                "Ted Waine",
                "Composite Viewport Layout Plugin")}));
}
}
