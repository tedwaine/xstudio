// SPDX-License-Identifier: Apache-2.0
#include "ocio_plugin.hpp"
#include "ocio_shared_settings.hpp"

#include "xstudio/utility/string_helpers.hpp"
#include "xstudio/ui/opengl/shader_program_base.hpp"

using namespace xstudio::colour_pipeline::ocio;
using namespace xstudio;


namespace {
const static utility::Uuid PLUGIN_UUID{"b39d1e3d-58f8-475f-82c1-081a048df705"};
} // anonymous namespace


OCIOColourPipeline::OCIOColourPipeline(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : ColourPipeline(cfg, init_settings) {

    global_controls_ = system().registry().template get<caf::actor>(SharedOCIOControls::NAME());
    if (!global_controls_) {
        global_controls_ = spawn<SharedOCIOControls>(init_settings);
    }

    worker_pool_ = caf::actor_pool::make(
        system().dummy_execution_unit(),
        4,
        [&] { return system().spawn<OCIOEngineActor>(); },
        caf::actor_pool::round_robin());
    link_to(worker_pool_);

    setup_ui();

    send(global_controls_, global_ocio_controls_atom_v, caf::actor_cast<caf::actor>(this));
}

OCIOColourPipeline::~OCIOColourPipeline() { global_controls_ = caf::actor(); }

caf::message_handler OCIOColourPipeline::message_handler_extensions() {

    // here we add a message handler to recieve messages from the
    // SharedOCIOControls about bypass, preferred view etc.
    // These handlers are merged with the message handlers provded by the base
    // class.
    return caf::message_handler(
               {
                   [=](global_ocio_controls_atom, const utility::JsonStore &settings) {
                       if (settings.is_null())
                           return;
                       auto old_bypass             = settings_.colour_bypass_;
                       auto old_use_preferred_view = settings_.use_preferred_view_;

                       settings_.colour_bypass_ =
                           settings.value("colour_bypass", settings_.colour_bypass_);
                       settings_.use_preferred_view_ =
                           settings.value("use_preferred_view", settings_.use_preferred_view_);
                       settings_.preferred_view_ =
                           settings.value("preferred_view", settings_.preferred_view_);
                       settings_.exposure_ = settings.value("exposure", settings_.exposure_);
                       settings_.gamma_    = settings.value("gamma", settings_.gamma_);
                       settings_.saturation_ =
                           settings.value("saturation", settings_.saturation_);
                       settings_.channel_ = settings.value("channel", settings_.channel_);

                       if (settings_.colour_bypass_ != old_bypass) {
                           update_bypass(settings_.colour_bypass_);
                       }

                       if (settings_.use_preferred_view_ != old_use_preferred_view) {
                           if (settings_.use_preferred_view_) {
                               view_->set_role_data(module::Attribute::Enabled, false, false);
                           } else {
                               view_->set_role_data(module::Attribute::Enabled, true, false);
                           }
                           media_source_changed(
                               current_source_uuid_, current_source_colour_mgmt_metadata_);
                       }
                   },
                   [=](global_ocio_controls_atom atom,
                       const std::string &attr_name,
                       const std::string &attr_value,
                       const std::string &ocio_config) {
                       // when the user sets the display or view in another OCIOColourPipeline
                       // (i.e. in another viewport) the change is sent to the
                       // SharedOCIOControls which then messages all other  OCIOColourPipeline
                       // instances about the change so we can stay in sync
                       if (ocio_config == current_config_name_) {
                           if (attr_name == "view") {
                               view_->set_value(attr_value, false);
                           } else if (attr_name == "display") {
                               display_->set_value(attr_value, false);
                           }
                       }
                   } /*,
                    [=](
                        colour_pipe_display_data_atom,
                        const media::AVFrameID &media_ptr,
                        const std::string &display,
                        const std::string &view) -> result<ColourOperationDataPtr> {
                        //This message is sent from main OCIOColourPipeline to worker instaces
                        // of OCIOColourPipeline. The main instance that is connected to the
                        // xSTUDIO UI keeps track of the OCIO display & view but the workers
                        // are ignorant of this so we send explicit values for display and
                        // view for a worker to use to build the linear->display transform
                        auto rp = make_response_promise<ColourOperationDataPtr>();
                        linear_to_display_op_data(rp, media_ptr, display, view);
                        return rp;
                    }*/
               })
        .or_else(ColourPipeline::message_handler_extensions());
}

std::string OCIOColourPipeline::fast_display_transform_hash(const media::AVFrameID &media_ptr) {
    return OCIOEngine::compute_hash(media_ptr.params_) +
           (settings_.colour_bypass_
                ? "null"
                : display_->value() + view_->value() +
                      (settings_.use_preferred_view_ ? "G" : "") + settings_.preferred_view_);
}

void OCIOColourPipeline::linearise_op_data(
    caf::typed_response_promise<ColourOperationDataPtr> &rp,
    const media::AVFrameID &media_ptr) {

    rp.delegate(
        worker_pool_, colour_pipe_linearise_data_atom_v, media_ptr, settings_.colour_bypass_);
}

void OCIOColourPipeline::linear_to_display_op_data(
    caf::typed_response_promise<ColourOperationDataPtr> &rp,
    const media::AVFrameID &media_ptr) {
    rp.delegate(
        worker_pool_,
        colour_pipe_display_data_atom_v,
        media_ptr,
        display(),
        view(media_ptr),
        settings_.colour_bypass_);
}

std::string OCIOColourPipeline::display() const { return display_->value(); }

std::string OCIOColourPipeline::view(const media::AVFrameID &media_ptr) const {
    return settings_.use_preferred_view_
               ? OCIOEngine::preferred_view(media_ptr.params_, settings_.preferred_view_)
               : view_->value();
}

utility::JsonStore OCIOColourPipeline::update_shader_uniforms(
    const media_reader::ImageBufPtr &image, std::any &user_data) {

    utility::JsonStore uniforms;
    if (settings_.channel_ == "Red") {
        uniforms["show_chan"] = 1;
    } else if (settings_.channel_ == "Green") {
        uniforms["show_chan"] = 2;
    } else if (settings_.channel_ == "Blue") {
        uniforms["show_chan"] = 3;
    } else if (settings_.channel_ == "Alpha") {
        uniforms["show_chan"] = 4;
    } else if (settings_.channel_ == "Luminance") {
        uniforms["show_chan"] = 5;
    } else {
        uniforms["show_chan"] = 0;
    }

    // TODO: ColSci
    // Saturation is not managed by OCIO currently
    uniforms["saturation"] = settings_.saturation_;

    OCIOEngine::update_shader_uniforms(
        user_data, uniforms, settings_.exposure_, settings_.gamma_);

    return uniforms;
}

void OCIOColourPipeline::process_thumbnail(
    caf::typed_response_promise<thumbnail::ThumbnailBufferPtr> &rp,
    const media::AVFrameID &media_ptr,
    const thumbnail::ThumbnailBufferPtr &buf) {

    rp.delegate(
        worker_pool_,
        media_reader::process_thumbnail_atom_v,
        media_ptr,
        buf,
        display(),
        view(media_ptr));
}

void OCIOColourPipeline::extend_pixel_info(
    media_reader::PixelInfo &pixel_info, const media::AVFrameID &frame_id) {

    try {

        OCIOEngine::extend_pixel_info(
            pixel_info,
            frame_id,
            display(),
            view(frame_id),
            settings_.exposure_,
            settings_.gamma_,
            settings_.saturation_);

    } catch (const std::exception &e) {

        spdlog::warn("OCIOColourPipeline: Failed to compute pixel probe: {}", e.what());
    }
}

void OCIOColourPipeline::media_source_changed(
    const utility::Uuid &source_uuid, const utility::JsonStore &src_colour_mgmt_metadata) {

    current_source_uuid_                 = source_uuid;
    current_source_colour_mgmt_metadata_ = src_colour_mgmt_metadata;

    if (settings_.colour_bypass_)
        return;

    // If the OCIO config for the new media is new, we need to update the UI
    const bool need_ui_update =
        OCIOEngine::compute_hash(src_colour_mgmt_metadata) != last_update_hash_;

    if (need_ui_update) {
        populate_ui(src_colour_mgmt_metadata);
    }

    if (settings_.use_preferred_view_) {
        view_->set_value(
            OCIOEngine::preferred_view(src_colour_mgmt_metadata, settings_.preferred_view_),
            false);
    }

    std::string src_cs = OCIOEngine::detect_source_colourspace(src_colour_mgmt_metadata);
    if (!src_cs.empty()) {
        // Send info about source colourspace to the global controls actor
        // so that main menu can be updated
        send(
            global_controls_,
            global_ocio_controls_atom_v,
            "source_colour_space",
            src_cs.c_str());
    }

    send(
        global_controls_,
        utility::event_atom_v,
        playhead::media_source_atom_v,
        source_uuid,
        src_colour_mgmt_metadata);
}

void OCIOColourPipeline::attribute_changed(
    const utility::Uuid &attribute_uuid, const int role) {

    if (attribute_uuid == view_->uuid()) {

        // forward user changes to view to the global settings actor. This will
        // bounce the change back to all OCIOColourPipeline instances so
        // we can stay in sync
        send(
            global_controls_,
            global_ocio_controls_atom_v,
            "view",
            view_->value(),
            current_config_name_);

    } else if (attribute_uuid == display_->uuid()) {

        media_source_changed(current_source_uuid_, current_source_colour_mgmt_metadata_);

        update_views(display_->value());


        // forward user changes to view to the global settings actor. This will
        // bounce the change back to all OCIOColourPipeline instances so
        // we can stay in sync
        send(
            global_controls_,
            global_ocio_controls_atom_v,
            "display",
            display_->value(),
            current_config_name_);
    }

    redraw_viewport();
}


void OCIOColourPipeline::screen_changed(
    const std::string &name,
    const std::string &model,
    const std::string &manufacturer,
    const std::string &serialNumber) {

    const std::string monitor_name = manufacturer + " " + model;
    const std::string display =
        OCIOEngine::default_display(current_source_colour_mgmt_metadata_, monitor_name);

    auto menu_populated = [](module::StringChoiceAttribute *attr) {
        return attr->get_role_data<std::vector<std::string>>(module::Attribute::StringChoices)
                   .size() > 0;
    };

    if (menu_populated(display_)) {
        display_->set_value(display);
    }
    monitor_name_ = monitor_name;
}

void OCIOColourPipeline::connect_to_viewport(
    const std::string &viewport_name, const std::string &viewport_toolbar_name, bool connect) {

    Module::connect_to_viewport(viewport_name, viewport_toolbar_name, connect);

    // Here we can add attrs to show up in the viewer context menu (right click)
    std::string viewport_context_menu_model_name = viewport_name + "_context_menu";

    insert_menu_item(
        viewport_context_menu_model_name, "OCIO Display", "", 20.0f, display_, false);
    insert_menu_item(viewport_context_menu_model_name, "OCIO View", "", 21.0f, view_, false);
    insert_menu_item(viewport_context_menu_model_name, "", "", 22.0f, nullptr, true); // divider

    make_attribute_visible_in_viewport_toolbar(view_);
    make_attribute_visible_in_viewport_toolbar(display_);

    // the SharedOCIOControls actor instance needs to connect to the viewport
    // too so it can expose its controls in the viewport toolbar etc.
    send(
        global_controls_,
        colour_pipeline::connect_to_viewport_atom_v,
        viewport_name,
        viewport_toolbar_name,
        connect);
}

void OCIOColourPipeline::setup_ui() {
    // OCIO Source colour space


    // OCIO display selection (main viewer)

    display_ = add_string_choice_attribute(ui_text_.DISPLAY, ui_text_.DISPLAY_SHORT);
    display_->set_redraw_viewport_on_change(true);
    display_->set_role_data(
        module::Attribute::Groups, nlohmann::json{"colour_pipe_attributes"});
    display_->set_role_data(module::Attribute::Enabled, true);
    display_->set_role_data(module::Attribute::ToolbarPosition, 10.0f);
    display_->set_role_data(module::Attribute::ToolTip, ui_text_.DISPLAY_TOOLTIP);

    // OCIO view selection

    view_ = add_string_choice_attribute(ui_text_.VIEW, ui_text_.VIEW);
    view_->set_redraw_viewport_on_change(true);
    view_->set_role_data(module::Attribute::Enabled, true);
    view_->set_role_data(module::Attribute::ToolbarPosition, 11.0f);
    view_->set_role_data(module::Attribute::ToolTip, ui_text_.VIEW_TOOLTIP);
}


void OCIOColourPipeline::populate_ui(const utility::JsonStore &src_colour_mgmt_metadata) {

    const auto config_name = OCIOEngine::get_ocio_config_name(src_colour_mgmt_metadata);

    if (current_config_name_ != config_name) {

        current_config_name_ = config_name;

        OCIOEngine::get_ocio_displays_view_colourspaces(
            src_colour_mgmt_metadata, all_colourspaces_, displays_, display_views_);

        // Config has changed, so update views and displays
        display_->set_role_data(module::Attribute::StringChoices, displays_, false);
        send(
            global_controls_,
            global_ocio_controls_atom_v,
            "source_colour_space",
            all_colourspaces_);

        std::string display, view;

        // Check if the global ocio pipeline actor has recorded the last
        // user change to display/view for this config
        try {

            scoped_actor sys{system()};

            auto config_settings = utility::request_receive<utility::JsonStore>(
                *sys, global_controls_, global_ocio_controls_atom_v, current_config_name_);

            if (!config_settings.is_null()) {
                display = config_settings.value("display", "");
                view    = config_settings.value("view", "");
            }

        } catch (std::exception &e) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
        }

        if (display.empty()) {
            display = OCIOEngine::default_display(src_colour_mgmt_metadata, monitor_name_);
        }
        if (view.empty()) {
            view = OCIOEngine::preferred_view(
                src_colour_mgmt_metadata,
                settings_.use_preferred_view_ ? settings_.preferred_view_ : "Default");
        }

        // 'false' flags mean we don't notify and attribute_changed is not called
        display_->set_value(display, false);
        view_->set_role_data(module::Attribute::StringChoices, display_views_[display], false);
        view_->set_value(view, false);
    }
}


void OCIOColourPipeline::update_views(const std::string new_display) {

    const std::vector<std::string> &new_views = display_views_[new_display];
    view_->set_role_data(module::Attribute::StringChoices, new_views, false);
    // Check whether the current view is available under the new display or not.
    const std::string curr_view = view_->value();
    bool has_curr_view =
        !new_views.empty() &&
        std::find(new_views.begin(), new_views.end(), curr_view) != new_views.end();

    if (!has_curr_view) {
        /*std::string default_view = ocio_config->getDefaultView(new_display.c_str());
        if (!default_view.empty()) {
            view_->set_value(default_view, false);
        }*/
    }
}

void OCIOColourPipeline::update_bypass(bool bypass) {

    view_->set_role_data(module::Attribute::Enabled, !bypass, false);
    display_->set_role_data(module::Attribute::Enabled, !bypass, false);
    if (bypass) {
        view_->set_value("N/A", false);
        display_->set_value("N/A", false);
    } else {
        media_source_changed(current_source_uuid_, current_source_colour_mgmt_metadata_);
    }
}

extern "C" {
plugin_manager::PluginFactoryCollection *plugin_factory_collection_ptr() {
    return new plugin_manager::PluginFactoryCollection(
        std::vector<std::shared_ptr<plugin_manager::PluginFactory>>(
            {std::make_shared<plugin_manager::PluginFactoryTemplate<OCIOColourPipeline>>(
                PLUGIN_UUID,
                "OCIOColourPipeline",
                plugin_manager::PluginFlags::PF_COLOUR_MANAGEMENT,
                false,
                "xStudio",
                "OCIO (v2) Colour Pipeline",
                semver::version("1.0.0"))}));
}
}