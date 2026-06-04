// SPDX-License-Identifier: Apache-2.0
#include <caf/actor_registry.hpp>

#include "mask_renderer_plugin.hpp"
#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/media_reader/image_buffer.hpp"
#include "xstudio/global_store/global_store.hpp"
#include "xstudio/utility/blind_data.hpp"
#include "xstudio/ui/viewport/viewport_helpers.hpp"
#include "xstudio/utility/helpers.hpp"

#include "mask_renderer.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;

MaskRendererPlugin::MaskRendererPlugin(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : plugin::StandardPlugin(cfg, "MaskRenderer", init_settings) {

    globally_enabled_ = add_boolean_attribute("Globally Enabled", "Globally Enabled", false);

    // register the plugin so that other plugins can find it and send it mask data to render
    system().registry().put(mask_renderer_registry, this);
}

MaskRendererPlugin::~MaskRendererPlugin() = default;

void MaskRendererPlugin::on_exit() {
    embedded_python_actor_ = caf::actor();
    system().registry().erase(mask_renderer_registry);
}

caf::message_handler MaskRendererPlugin::message_handler_extensions() {

    // this is where we get details on masks to render from other plugins.

    return caf::message_handler(
               {[=](ui::viewport::hud_settings_atom, bool hud_enabled) {
                    globally_enabled_->set_value(hud_enabled);
                    redraw_viewport();
                },
                /*[=](ui::viewport::hud_settings_atom,
                    const std::string qml_code,
                    HUDElementPosition position) -> bool {
                    return false;
                },*/
                [=](viewport_mask_atom,
                    const utility::Uuid &plugin_uuid,
                    const std::string plugin_name,
                    caf::actor plugin_actor,
                    const bool masks_different_per_media) {
                    // This message is sent from the constructor of the MaskPlugin pythgon
                    // base class.

                    if (plugin_actor && masks_different_per_media) {
                        // we only need to have handles on maks plugins that define
                        // potentially different masks for different bits of media.
                        // For mask plugins that define only a static mask the plugin
                        // will send us the Mask details when it changes.
                        media_masks_[plugin_uuid] = std::map<utility::Uuid, MaskPtrVecPtr>();
                    }

                    if (!embedded_python_actor_) {
                        embedded_python_actor_ = system().registry().template get<caf::actor>(
                            xstudio::embedded_python_registry);
                    }

                    if (!masks_different_per_media && embedded_python_actor_) {
                        // fetch static mask(s) from the plugin
                        mail(
                            embedded_python::python_exec_atom_v,
                            ui::viewport::viewport_mask_atom_v,
                            plugin_uuid,
                            caf::actor())
                            .request(embedded_python_actor_, infinite)
                            .then(
                                [=](const std::vector<Mask> &masks) {
                                    static_masks_[plugin_uuid] = MaskPtrVecPtr(masks);
                                },
                                [=](caf::error &err) {

                                });
                    }
                },

                [=](viewport_mask_atom, const utility::Uuid &plugin_uuid, const bool enabled) {
                    if (!enabled) {
                        redraw_viewport();
                    }
                },

                [=](viewport_mask_atom,
                    utility::change_atom,
                    const utility::Uuid &plugin_uuid) {
                    // this message is sent by a MaskPlugin python plugin when one of it's
                    // attributes changes. we need to redraw the viewport because the mask
                    // details may have changed.
                    auto p = media_masks_.find(plugin_uuid);
                    if (p != media_masks_.end()) {
                        // if this is a plugin that defines different masks for different media
                        // sources we need to clear the cache of masks we have for each media
                        // source because they may now be out of date.
                        p->second.clear();
                    }
                    redraw_viewport();
                },

                [=](viewport_mask_atom,
                    const utility::Uuid &plugin_uuid,
                    const std::vector<Mask> &static_masks) {
                    static_masks_[plugin_uuid] = MaskPtrVecPtr(static_masks);
                    redraw_viewport();
                }})
        .or_else(StandardPlugin::message_handler_extensions());
}

void MaskRendererPlugin::on_screen_media_changed(
    const utility::UuidActor &media,
    const utility::UuidActor &media_source,
    const std::string &viewport_name,
    const int playhead_idx,
    const bool is_main_playhead) {

    if (is_main_playhead && globally_enabled_->value() && playhead_idx == 0) {
        // mask plugins that define different masks for different media sources
        // might want to know which media source is now on screen so it can
        // modify it's configuration options accordingly.
        for (auto &mp : media_masks_) {
            // mp.first is the UUID of the Python plugin that provides masks
            // that may vary per media source.
            mail(
                embedded_python::python_exec_atom_v,
                ui::viewport::viewport_mask_atom_v,
                playhead::show_atom_v,
                media_source.actor(),
                mp.first)
                .send(embedded_python_actor_);
        }
    }
}

void MaskRendererPlugin::media_due_on_screen_soon(
    const media::AVFrameIDsAndTimePoints &frame_ids) {

    // this callback is made during playback - it gives us a list of frame IDs, one for each
    // individual piece of media that is expected to be put on the screen in the immediate
    // future

    // there is one entry in media_masks_ PER PLUGIN (if the mask plugin defines
    // different masks for different media sources). As such, there is only
    // likely to be one of these plugins as most VFX/ANIM studios would have
    // a single plugin implementation to define masks for the xSTUDIO viewport.
    for (auto &mp : media_masks_) {

        const auto plugin_id = mp.first;

        utility::UuidActorVector media_source_actors;

        // loop over the incoming frameIds ... find the ones that we haven't
        // already requested masks for
        for (const auto &p : frame_ids) {

            if (!mp.second[p.second->source_uuid()] &&
                !mp.second[p.second->source_uuid()].in_flight) {
                // add an entry for this media source so we don't re-attempt to
                // fetch the mask data.
                mp.second[p.second->source_uuid()].in_flight = true;
                media_source_actors.push_back(p.second->media_source_actor());
            }
        }

        // we haven't got mask data for this media source yet. Request it from the python
        // plugin(s).
        mail(
            embedded_python::python_exec_atom_v,
            ui::viewport::viewport_mask_atom_v,
            mp.first,
            media_source_actors)
            .request(embedded_python_actor_, std::chrono::milliseconds(1000))
            .then(
                [=](const std::vector<std::vector<Mask>> &masks) mutable {
                    if (masks.size() != media_source_actors.size()) {
                        spdlog::warn(
                            "{} {}",
                            __PRETTY_FUNCTION__,
                            "Number of masks returned from python plugin doesn't match number "
                            "of media sources requested");
                        for (auto &p2 : media_source_actors) {
                            media_masks_[plugin_id][p2.uuid()].in_flight = false;
                        }
                        return;
                    }
                    auto p2 = media_source_actors.begin();
                    for (const auto &mask_vec : masks) {
                        MaskPtrVec mvec;
                        for (const auto &m : mask_vec) {
                            mvec.emplace_back(std::make_shared<const Mask>(m));
                        }
                        media_masks_[plugin_id][(*p2).uuid()] = MaskPtrVecPtr(mvec);
                        p2++;
                    }
                },
                [=](caf::error &err) mutable {
                    for (auto &p2 : media_source_actors) {
                        media_masks_[plugin_id][p2.uuid()].in_flight = false;
                    }
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                });
    }
}


utility::BlindDataObjectPtr MaskRendererPlugin::onscreen_render_data(
    const media_reader::ImageBufPtr &image,
    const std::string &viewport_name,
    const utility::Uuid &playhead_uuid,
    const bool is_hero_image,
    const bool images_are_in_grid_layout) const {

    auto data = new MaskData();

    for (const auto &p : static_masks_) {
        if (p.second) {
            data->insert(data->end(), p.second->begin(), p.second->end());
        }
    }

    for (auto &mp : media_masks_) {

        MaskPtrVecPtr &media_masks = mp.second[image.frame_id().source_uuid()];

        if (!media_masks) {
            // we haven't got mask data for this media source yet. Request it from the python
            // plugin(s). This is a synchronous request and remember that onscreen_render_data
            // is called in the critical path of rendering the viewport, so we want to avoid
            // doing this if possible. That's why we also do this request in
            // media_due_on_screen_soon - ideally we will have the data by the time we get here.
            // If we need to do this we use a short timeout s if the python plugin is slow.
            try {

                const auto masks = utility::request_receive_wait<std::vector<Mask>>(
                    *(scoped_actor{system()}),
                    embedded_python_actor_,
                    std::chrono::milliseconds(10),
                    embedded_python::python_exec_atom_v,
                    ui::viewport::viewport_mask_atom_v,
                    mp.first,
                    image.frame_id().media_source_actor().actor());

                media_masks = MaskPtrVecPtr(masks);

            } catch (std::exception &e) {
                spdlog::debug("{} {}", __PRETTY_FUNCTION__, e.what());
            }
        }

        if (media_masks) {
            data->insert(data->end(), media_masks->begin(), media_masks->end());
        }
    }

    return utility::BlindDataObjectPtr(data);
}

plugin::ViewportOverlayRendererPtr
MaskRendererPlugin::make_overlay_renderer(const std::string &viewport_name) {
    return plugin::ViewportOverlayRendererPtr(new MaskRenderer());
}


XSTUDIO_PLUGIN_DECLARE_BEGIN()

XSTUDIO_REGISTER_PLUGIN(
    MaskRendererPlugin,
    MaskRendererPlugin::PLUGIN_UUID,
    Mask Renderer Plugin,
    plugin_manager::PluginFlags::PF_HEAD_UP_DISPLAY | plugin_manager::PluginFlags::PF_VIEWPORT_OVERLAY,
    true,
    Ted Waine,
    Plugin that renders masks in the viewport - this can be used by other plugins that provide mask info for media sources going on-screen,
    1.0.0)

XSTUDIO_PLUGIN_DECLARE_END()
