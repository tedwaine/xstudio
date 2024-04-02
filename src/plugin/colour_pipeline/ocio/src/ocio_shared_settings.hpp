// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/plugin_manager/plugin_base.hpp"
#include "ui_text.hpp"

namespace xstudio {
namespace colour_pipeline {

    /* Our model for OCIO colour management requires some global controls. We
    want to toggle colour management on/off, we want to choose whether the OCIO
    view is automatically selected according to our own rules based on the source
    media (its metadata) or whether the user will set the view themselves. We
    also want a global menu to set the source colourspace of the on-screen media.
    As there are multiple instances of the OCIOColourPipeline (one for each viewport,
    of which there can be several) we create a singleton class that keeps track of
    the global settings and transmits the state of these to all OCIOColourPipeline
    instances.*/
    class SharedOCIOControls : public plugin::StandardPlugin {

      public:
        explicit SharedOCIOControls(
            caf::actor_config &cfg, const utility::JsonStore &init_settings);

        ~SharedOCIOControls();

        void attribute_changed(const utility::Uuid &attribute_uuid, const int role) override;

        caf::message_handler message_handler_extensions() override;

        void on_exit() override;

        void register_hotkeys() override;

        void
        hotkey_pressed(const utility::Uuid &hotkey_uuid, const std::string &context) override;

        void
        hotkey_released(const utility::Uuid &hotkey_uuid, const std::string &context) override;

        bool pointer_event(const ui::PointerEvent &e) override;

        void connect_to_viewport(
            const std::string &viewport_name,
            const std::string &viewport_toolbar_name,
            bool connect) override;


        inline static std::string NAME() { return "OCIO_GLOBAL_CONTROLS"; }

      private:
        void set_media_source_colourspace(
            const utility::Uuid &media_uuid, const std::string &colourspace);

        void update_settings();

        module::BooleanAttribute *colour_bypass_;
        module::StringChoiceAttribute *preferred_view_;
        module::BooleanAttribute *use_preferred_view_;
        module::StringChoiceAttribute *source_colour_space_;

        module::StringChoiceAttribute *channel_;
        module::FloatAttribute *exposure_;
        module::FloatAttribute *gamma_;
        module::FloatAttribute *saturation_;
        module::BooleanAttribute *enable_saturation_;
        module::BooleanAttribute *enable_gamma_;

        std::map<utility::Uuid, std::string> channel_hotkeys_;
        utility::Uuid exposure_hotkey_;
        utility::Uuid gamma_hotkey_;
        utility::Uuid saturation_hotkey_;
        utility::Uuid reset_hotkey_;

        UiText ui_text_;
        std::vector<caf::actor> watchers_;
        utility::Uuid onscreen_media_id_;
        utility::JsonStore sync_settings_;

        utility::JsonStore settings_data_;
    };

} // namespace colour_pipeline
} // namespace xstudio
