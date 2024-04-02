// SPDX-License-Identifier: Apache-2.0
#include "ocio_shared_settings.hpp"

#include "xstudio/utility/string_helpers.hpp"
#include "xstudio/utility/helpers.hpp"

using namespace xstudio::colour_pipeline;
using namespace xstudio;

SharedOCIOControls::SharedOCIOControls(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : StandardPlugin(cfg, "SharedOCIOControls", init_settings) {

    system().registry().put(SharedOCIOControls::NAME(), this);

    // make sure we get cleaned up when the global actor exits
    link_to(system().registry().template get<caf::actor>(global_registry));

    // Colour bypass
    colour_bypass_ = add_boolean_attribute(ui_text_.CMS_OFF, ui_text_.CMS_OFF_SHORT, false);

    colour_bypass_->set_redraw_viewport_on_change(true);
    colour_bypass_->set_role_data(
        module::Attribute::Groups, nlohmann::json{"colour_pipe_attributes"});
    colour_bypass_->set_role_data(module::Attribute::Enabled, false);
    colour_bypass_->set_role_data(module::Attribute::ToolTip, ui_text_.CS_BYPASS_TOOLTIP);
    // 'colour bypass' is a global setting.
    colour_bypass_->set_role_data(
        module::Attribute::UuidRole, utility::Uuid("222902ee-167b-4c74-91aa-04eb74fd4357"));


    // Preferred view
    preferred_view_ = add_string_choice_attribute(
        ui_text_.PREF_VIEW, ui_text_.PREF_VIEW, ui_text_.DEFAULT_VIEW);
    preferred_view_->set_redraw_viewport_on_change(true);
    preferred_view_->set_role_data(module::Attribute::Enabled, false);
    preferred_view_->set_role_data(module::Attribute::ToolbarPosition, 11.0f);
    preferred_view_->set_role_data(module::Attribute::ToolTip, ui_text_.PREF_VIEW_TOOLTIP);
    preferred_view_->set_role_data(
        module::Attribute::StringChoices, ui_text_.PREF_VIEW_OPTIONS, false);
    preferred_view_->set_preference_path("/plugin/colour_pipeline/ocio/preferred_view");

    // View mode

    use_preferred_view_ =
        add_boolean_attribute(ui_text_.VIEW_MODE, ui_text_.GLOBAL_VIEW_SHORT, false);
    use_preferred_view_->set_redraw_viewport_on_change(true);
    use_preferred_view_->set_role_data(
        module::Attribute::Groups, nlohmann::json{"colour_pipe_attributes"});
    use_preferred_view_->set_role_data(
        module::Attribute::ToolTip, ui_text_.GLOBAL_VIEW_TOOLTIP);
    use_preferred_view_->set_preference_path("/plugin/colour_pipeline/ocio/use_preferred_view");

    // colour space setting and option for on-screen source for the 'key'
    // playhead/viewport.... xStudio can have multiple viewports showing
    // different media at the same time. There is a global 'key' playhead,
    // however, which in most cases is what is delivering images to viewports
    // in the main xStudio interface. The current media for the key playhead is
    // what is tracked and changed by 'source_colour_space_'
    source_colour_space_ =
        add_string_choice_attribute(ui_text_.SOURCE_CS, ui_text_.SOURCE_CS_SHORT);
    source_colour_space_->set_role_data(module::Attribute::ToolbarPosition, 12.0f);
    source_colour_space_->set_role_data(module::Attribute::ToolTip, ui_text_.SOURCE_CS_TOOLTIP);

    // Hot channel selection

    channel_ = add_string_choice_attribute(
        ui_text_.CHANNEL,
        ui_text_.CHANNEL_SHORT,
        ui_text_.RGB,
        {ui_text_.RGB,
         ui_text_.RED,
         ui_text_.GREEN,
         ui_text_.BLUE,
         ui_text_.ALPHA,
         ui_text_.LUMINANCE},
        {ui_text_.RGB, ui_text_.R, ui_text_.G, ui_text_.B, ui_text_.A, ui_text_.L});
    channel_->set_redraw_viewport_on_change(true);
    channel_->set_role_data(module::Attribute::Enabled, true);
    channel_->set_role_data(module::Attribute::ToolbarPosition, 8.0f);
    channel_->set_role_data(module::Attribute::ToolTip, ui_text_.CS_MSG_CMS_SELECT_CLR_TIP);

    // Exposure slider

    exposure_ = add_float_attribute(
        ui_text_.EXPOSURE, ui_text_.EXPOSURE_SHORT, 0.0f, -10.0f, 10.0f, 0.05f);
    exposure_->set_redraw_viewport_on_change(true);
    exposure_->set_role_data(module::Attribute::ToolbarPosition, 4.0f);
    exposure_->set_role_data(module::Attribute::Activated, false);
    exposure_->set_role_data(module::Attribute::DefaultValue, 0.0f);
    exposure_->set_role_data(module::Attribute::ToolTip, ui_text_.CS_MSG_CMS_SET_EXP_TIP);

    // Gamma slider

    gamma_ = add_float_attribute(ui_text_.GAMMA, ui_text_.GAMMA_SHORT, 1.0f, 0.0f, 5.0f, 0.05f);
    gamma_->set_redraw_viewport_on_change(true);
    gamma_->set_role_data(module::Attribute::ToolbarPosition, 4.1f);
    gamma_->set_role_data(module::Attribute::Activated, false);
    gamma_->set_role_data(module::Attribute::DefaultValue, 1.0f);
    gamma_->set_role_data(module::Attribute::ToolTip, ui_text_.CS_MSG_CMS_SET_GAMMA_TIP);

    enable_gamma_ =
        add_boolean_attribute(ui_text_.ENABLE_GAMMA, ui_text_.ENABLE_GAMMA_SHORT, false);
    enable_gamma_->set_redraw_viewport_on_change(true);
    enable_gamma_->set_preference_path("/plugin/colour_pipeline/ocio/enable_gamma");

    // Saturation slider

    saturation_ = add_float_attribute(
        ui_text_.SATURATION, ui_text_.SATURATION_SHORT, 1.0f, 0.0f, 10.0f, 0.05f);
    saturation_->set_redraw_viewport_on_change(true);
    saturation_->set_role_data(module::Attribute::ToolbarPosition, 4.2f);
    saturation_->set_role_data(module::Attribute::Activated, false);
    saturation_->set_role_data(module::Attribute::DefaultValue, 1.0f);
    saturation_->set_role_data(
        module::Attribute::ToolTip, ui_text_.CS_MSG_CMS_SET_SATURATION_TIP);

    enable_saturation_ = add_boolean_attribute(
        ui_text_.ENABLE_SATURATION, ui_text_.ENABLE_SATURATION_SHORT, false);
    enable_saturation_->set_redraw_viewport_on_change(true);
    enable_saturation_->set_preference_path("/plugin/colour_pipeline/ocio/enable_saturation");

    // we need to call this base class method before calling insert_menu_item
    make_behavior();

    insert_menu_item(
        "main menu bar", "Bypass Colour Management", "Colour", 1.0f, colour_bypass_, false);
    insert_menu_item(
        "main menu bar", "Use Preferred OCIO View", "Colour", 2.0f, use_preferred_view_, false);
    insert_menu_item(
        "main menu bar", "OCIO Preferred View", "Colour", 3.0f, preferred_view_, false);
    insert_menu_item(
        "main menu bar",
        "On-Screen Media Colourspace",
        "Colour",
        4.0f,
        source_colour_space_,
        false);

    // make sure the colour menu appears in the right place in the main menu bar.
    set_submenu_position_in_parent(
        "main menu bar",
        "Colour",
        30.0f);

    make_attribute_visible_in_viewport_toolbar(exposure_);
    make_attribute_visible_in_viewport_toolbar(channel_);
    make_attribute_visible_in_viewport_toolbar(gamma_);
    make_attribute_visible_in_viewport_toolbar(saturation_);

    // insert_menu_item(viewport_context_menu_model_name, "Channel", "", 8.5f, channel_, false);

    set_down_handler([=](down_msg &msg) {
        auto p = watchers_.begin();
        while (p != watchers_.end()) {
            if (msg.source == *p)
                p = watchers_.erase(p);
            else
                p++;
        }
    });

    update_settings();
    connect_to_ui();
}

SharedOCIOControls::~SharedOCIOControls() {}

void SharedOCIOControls::register_hotkeys() {

    for (const auto &hotkey_props : ui_text_.channel_hotkeys) {
        auto hotkey_id = register_hotkey(
            hotkey_props.key,
            hotkey_props.modifier,
            hotkey_props.name,
            hotkey_props.description);

        channel_hotkeys_[hotkey_id] = hotkey_props.channel_name;
    }

    reset_hotkey_ = register_hotkey(
        int('R'),
        ui::ControlModifier,
        "Reset Colour Viewing Setting",
        "Resets viewer exposure and channel mode");

    exposure_hotkey_ = register_hotkey(
        int('E'),
        ui::NoModifier,
        "Exposure Scrubbing",
        "Hold this key down and click-scrub the mouse pointer left/right in the viewport to "
        "adjust viewer exposure");

    gamma_hotkey_ = register_hotkey(
        int('Y'),
        ui::NoModifier,
        "Gamma Scrubbing",
        "Hold this key down and click-scrub the mouse pointer left/right in the viewport to "
        "adjust viewer gamma");

    saturation_hotkey_ = register_hotkey(
        int('S'),
        ui::AltModifier,
        "Saturation Scrubbing",
        "Hold this key down and click-scrub the mouse pointer left/right in the viewport to "
        "adjust viewer saturation");
}

void SharedOCIOControls::on_exit() {
    system().registry().erase(SharedOCIOControls::NAME());
    watchers_.clear();
}

caf::message_handler SharedOCIOControls::message_handler_extensions() {

    // here we add a message handler to recieve messages from the
    // SharedOCIOControls about bypass, preferred view etc.
    // These handlers are merged with the message handlers provded by the base
    // class.
    return caf::message_handler(
               {[=](connect_to_viewport_atom,
                    const std::string &viewport_name,
                    const std::string &viewport_toolbar_name,
                    bool connect) {
                    connect_to_viewport(viewport_name, viewport_toolbar_name, connect);
                },
                [=](global_ocio_controls_atom, caf::actor watcher) {
                    monitor(watcher);
                    watchers_.push_back(watcher);
                    send(watcher, global_ocio_controls_atom_v, settings_data_);
                },
                [=](utility::event_atom,
                    playhead::media_source_atom,
                    const utility::Uuid &source_uuid,
                    const utility::JsonStore & /*src_colour_mgmt_metadata*/) {
                    onscreen_media_id_ = source_uuid;
                },
                [=](global_ocio_controls_atom,
                    const std::string &attr_name,
                    const std::string &string_value) {
                    if (attr_name == "source_colour_space") {
                        source_colour_space_->set_value(string_value, false);
                    }
                },
                [=](global_ocio_controls_atom,
                    const std::string &attr_name,
                    const std::vector<std::string> &choices) {
                    if (attr_name == "source_colour_space") {
                        source_colour_space_->set_role_data(
                            module::Attribute::StringChoices, choices, false);
                    }
                },
                [=](global_ocio_controls_atom atom,
                    const std::string &attr_name,
                    const std::string &attr_value,
                    const std::string &ocio_config) {
                    // when certain attributes change (like view or display) on one
                    // OCIOColourPipeline instance, we want to sync to other instances.
                    // Here we simply forward the message back to all instances. The
                    // ocio_config is used to determine whether the instance is running
                    // the same config and should therefore sync
                    for (auto &watcher : watchers_) {
                        send(watcher, atom, attr_name, attr_value, ocio_config);
                    }
                    // store the attr setting so that new OCIOColourPipeline instances
                    // are able to sync
                    sync_settings_[ocio_config][attr_name] = attr_value;
                },
                [=](global_ocio_controls_atom atom,
                    const std::string &ocio_config) -> utility::JsonStore {
                    // this message handler lets an OCIOColourPipeline check if the
                    // display/view has been set for a given ocio_config so it can use
                    // those values to initialise itself
                    if (sync_settings_.contains(ocio_config)) {
                        return sync_settings_[ocio_config];
                    }
                    return utility::JsonStore();
                }})
        .or_else(StandardPlugin::message_handler_extensions());
}

void SharedOCIOControls::hotkey_pressed(
    const utility::Uuid &hotkey_uuid, const std::string &context) {

    // If user hits 'R' hotkey and we're already looking at the red channel,
    // then we revert back to RGB, same for 'G' and 'B'.
    auto p = channel_hotkeys_.find(hotkey_uuid);
    if (p != channel_hotkeys_.end()) {
        if (channel_->value() == p->second) {
            channel_->set_value("RGB");
        } else {
            channel_->set_value(p->second);
        }
    } else if (hotkey_uuid == reset_hotkey_) {
        channel_->set_value("RGB");
        exposure_->set_value(exposure_->get_role_data<float>(module::Attribute::DefaultValue));
        gamma_->set_value(gamma_->get_role_data<float>(module::Attribute::DefaultValue));
        saturation_->set_value(
            saturation_->get_role_data<float>(module::Attribute::DefaultValue));
    } else if (hotkey_uuid == exposure_hotkey_) {
        exposure_->set_role_data(module::Attribute::Activated, true);
        grab_mouse_focus();
    } else if (hotkey_uuid == gamma_hotkey_) {
        gamma_->set_role_data(module::Attribute::Activated, true);
        grab_mouse_focus();
    } else if (hotkey_uuid == saturation_hotkey_) {
        saturation_->set_role_data(module::Attribute::Activated, true);
        grab_mouse_focus();
    }
}

void SharedOCIOControls::hotkey_released(
    const utility::Uuid &hotkey_uuid, const std::string &context) {

    if (hotkey_uuid == exposure_hotkey_) {
        exposure_->set_role_data(module::Attribute::Activated, false);
        release_mouse_focus();
    } else if (hotkey_uuid == gamma_hotkey_) {
        gamma_->set_role_data(module::Attribute::Activated, false);
        release_mouse_focus();
    } else if (hotkey_uuid == saturation_hotkey_) {
        saturation_->set_role_data(module::Attribute::Activated, false);
        release_mouse_focus();
    }
}

bool SharedOCIOControls::pointer_event(const ui::PointerEvent &e) {

    module::FloatAttribute *active_attr = nullptr;
    if (exposure_->get_role_data<bool>(module::Attribute::Activated)) {
        active_attr = exposure_;
    } else if (gamma_->get_role_data<bool>(module::Attribute::Activated)) {
        active_attr = gamma_;
    } else if (saturation_->get_role_data<bool>(module::Attribute::Activated)) {
        active_attr = saturation_;
    }

    // Nothing to be done
    if (!active_attr) {
        return false;
    }

    // Implementing exposure scrubbing in viewport
    static int x_down;
    static float e_down;
    static auto dragging = false;
    bool used            = false;

    if (e.type() == ui::Signature::EventType::ButtonDown &&
        e.buttons() == ui::Signature::Left) {
        x_down   = e.x();
        e_down   = active_attr->value();
        dragging = true;
        used     = true;
    } else if (dragging && e.buttons() == ui::Signature::Left) {
        const auto sensitivity =
            active_attr->get_role_data<float>(module::Attribute::FloatScrubSensitivity);
        const auto step = active_attr->get_role_data<float>(module::Attribute::FloatScrubStep);
        const auto min  = active_attr->get_role_data<float>(module::Attribute::FloatScrubMin);
        const auto max  = active_attr->get_role_data<float>(module::Attribute::FloatScrubMax);

        auto val = 0.0f;
        val      = round((e_down + (e.x() - x_down) * sensitivity) / step) * step;
        val      = std::max(std::min(val, max), min);
        active_attr->set_value(val);
        used = true;
    } else if (
        e.type() == ui::Signature::EventType::ButtonRelease &&
        (e.buttons() & ui::Signature::Left)) {
        dragging = false;
        used     = true;
    }

    if (e.type() == ui::Signature::EventType::DoubleClick) {
        static auto last_value = 0.0f;
        const auto def = active_attr->get_role_data<float>(module::Attribute::DefaultValue);

        if (active_attr->value() == def) {
            active_attr->set_value(last_value);
        } else {
            last_value = active_attr->value();
            active_attr->set_value(def);
        }
        used = true;
    }
    return used;
}

void SharedOCIOControls::connect_to_viewport(
    const std::string &viewport_name, const std::string &viewport_toolbar_name, bool connect) {
    Module::connect_to_viewport(viewport_name, viewport_toolbar_name, connect);
    std::string viewport_context_menu_model_name = viewport_name + "_context_menu";
    insert_menu_item(viewport_context_menu_model_name, "Channel", "", 21.5f, channel_, false);
}

void SharedOCIOControls::attribute_changed(
    const utility::Uuid &attribute_uuid, const int role) {

    if (role == module::Attribute::Value && attribute_uuid == source_colour_space_->uuid()) {
        // user has changed the media source colour space. We need modify the source
        // metadata to reflect this
        set_media_source_colourspace(onscreen_media_id_, source_colour_space_->value());
    } else {
        // one of our settings attributes has changed (exposure, gamma etc..)
        update_settings();

        if (attribute_uuid == use_preferred_view_->uuid()) {
            preferred_view_->set_role_data(
                module::Attribute::Enabled, use_preferred_view_->value());
        }
    }
    plugin::StandardPlugin::attribute_changed(attribute_uuid, role);
}

void SharedOCIOControls::set_media_source_colourspace(
    const utility::Uuid &source_uuid, const std::string &colourspace) {

    try {

        scoped_actor sys{system()};

        // first, get to the session
        auto session = utility::request_receive<caf::actor>(
            *sys,
            system().registry().template get<caf::actor>(global_registry),
            session::session_atom_v);

        // get the session to search its playlists for the media source
        auto media_source_actor = utility::request_receive<caf::actor>(
            *sys, session, media::get_media_source_atom_v, source_uuid);

        if (media_source_actor) {
            // here we can set the overidden source colourspace
            auto colour_data = utility::request_receive<bool>(
                *sys,
                media_source_actor,
                json_store::set_json_atom_v,
                utility::JsonStore(colourspace),
                "/colour_pipeline/override_input_cs");
        }

    } catch (std::exception &e) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
    }
}

void SharedOCIOControls::update_settings() {

    settings_data_["colour_bypass"]      = colour_bypass_->value();
    settings_data_["use_preferred_view"] = use_preferred_view_->value();
    settings_data_["preferred_view"]     = preferred_view_->value();
    settings_data_["exposure"]           = exposure_->value();
    settings_data_["gamma"]              = gamma_->value();
    settings_data_["saturation"]         = saturation_->value();
    settings_data_["channel"]            = channel_->value();

    // this syncs all the settings on the OCIOColourPipeline instances
    for (auto &watcher : watchers_) {
        send(watcher, global_ocio_controls_atom_v, settings_data_);
    }
}