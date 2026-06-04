// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/ui/viewport/mask.hpp"
#include "xstudio/plugin_manager/hud_plugin.hpp"

namespace xstudio::ui::viewport {

typedef std::vector<std::shared_ptr<const Mask>> MaskPtrVec;
class MaskPtrVecPtr : public std::shared_ptr<MaskPtrVec> {
  public:
    MaskPtrVecPtr() = default;

    MaskPtrVecPtr(const std::vector<Mask> &masks) {
        reset(new MaskPtrVec);
        for (const auto &m : masks) {
            (*this)->emplace_back(std::make_shared<const Mask>(m));
        }
    }
    MaskPtrVecPtr(const MaskPtrVec &mvec)
        : std::shared_ptr<MaskPtrVec>(std::make_shared<MaskPtrVec>(mvec)) {}
    MaskPtrVecPtr(const MaskPtrVecPtr &o)            = default;
    MaskPtrVecPtr &operator=(const MaskPtrVecPtr &o) = default;

    bool in_flight = false;
};

class MaskData : public utility::BlindDataObject, public MaskPtrVec {
  public:
    MaskData()  = default;
    ~MaskData() = default;
};

class MaskRendererPlugin : public plugin::StandardPlugin {

  public:
    inline static const utility::Uuid PLUGIN_UUID =
        utility::Uuid("d932fa83-2559-485b-9846-1f4e6c614fea");

    MaskRendererPlugin(caf::actor_config &cfg, const utility::JsonStore &init_settings);

    ~MaskRendererPlugin();

    void on_exit() override;

    caf::message_handler message_handler_extensions() override;

  protected:
    utility::BlindDataObjectPtr onscreen_render_data(
        const media_reader::ImageBufPtr &,
        const std::string & /*viewport_name*/,
        const utility::Uuid &playhead_uuid,
        const bool is_hero_image,
        const bool images_are_in_grid_layout) const override;

    plugin::ViewportOverlayRendererPtr make_overlay_renderer(const std::string &viewport_name) override;

    void media_due_on_screen_soon(const media::AVFrameIDsAndTimePoints &) override;

    void on_screen_media_changed(
        const utility::UuidActor &media,
        const utility::UuidActor &media_source,
        const std::string &viewport_name,
        const int playhead_idx,
        const bool is_main_playhead) override;

  private:
    std::map<utility::Uuid, utility::BlindDataObjectPtr> masks_per_media_cache_;

    module::BooleanAttribute *globally_enabled_ = {nullptr};

    caf::actor embedded_python_actor_;

    std::map<utility::Uuid, MaskPtrVecPtr> static_masks_;

    // key is the UUID of the plugin that provides the masks.
    // The key of the sub-map is the media source UUID and the value is the mask(s) for
    // that media source provided by that plugin.
    mutable std::map<utility::Uuid, std::map<utility::Uuid, MaskPtrVecPtr>> media_masks_;
};

} // namespace xstudio::ui::viewport
