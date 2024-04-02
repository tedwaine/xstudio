// SPDX-License-Identifier: Apache-2.0

#include <limits>

#include "xstudio/ui/opengl/shader_program_base.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/string_helpers.hpp"

#include "grading.h"
#include "grading_mask_render_data.h"
#include "grading_mask_gl_renderer.h"
#include "grading_colour_op.hpp"

using namespace xstudio;
using namespace xstudio::bookmark;
using namespace xstudio::colour_pipeline;
using namespace xstudio::ui::viewport;


namespace {

std::vector<float> array4d_to_vector4f(const std::array<double, 4> &arr) {
    return std::vector<float>{
        static_cast<float>(arr[0]),
        static_cast<float>(arr[1]),
        static_cast<float>(arr[2]),
        static_cast<float>(arr[3])};
}

std::array<double, 4> vector4f_to_array4d(const std::vector<float> &vec) {
    return std::array<double, 4>{vec[0], vec[1], vec[2], vec[3]};
}

} // anonymous namespace


GradingTool::GradingTool(caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : plugin::StandardPlugin(cfg, "GradingTool", init_settings) {

    // General

    tool_is_active_ =
        add_boolean_attribute("grading_tool_active", "grading_tool_active", false);
    tool_is_active_->expose_in_ui_attrs_group("grading_settings");
    tool_is_active_->set_role_data(
        module::Attribute::MenuPaths,
        std::vector<std::string>({"panels_main_menu_items|Grading Tool"}));

    grading_action_ = add_string_attribute("grading_action", "grading_action", "");
    grading_action_->expose_in_ui_attrs_group("grading_settings");

    grading_bypass_ = add_boolean_attribute("drawing_bypass", "drawing_bypass", false);
    grading_bypass_->expose_in_ui_attrs_group("grading_settings");

    drawing_action_ = add_string_attribute("drawing_action", "drawing_action", "");
    drawing_action_->expose_in_ui_attrs_group("grading_settings");
    drawing_action_->expose_in_ui_attrs_group("mask_tool_settings");

    grading_tracking_ = add_boolean_attribute("grading_tracking", "grading_tracking", false);
    grading_tracking_->expose_in_ui_attrs_group("grading_settings");
    grading_tracking_->set_preference_path("/plugin/grading/grading_tracking");

    media_colour_managed_ =
        add_boolean_attribute("media_colour_managed", "media_colour_managed", false);
    media_colour_managed_->expose_in_ui_attrs_group("grading_settings");

    tool_panel_ = add_string_choice_attribute(
        "tool_panel",
        "tool_panel",
        utility::map_value_to_vec(tool_panel_names_).front(),
        utility::map_value_to_vec(tool_panel_names_));
    tool_panel_->expose_in_ui_attrs_group("grading_settings");
    tool_panel_->expose_in_ui_attrs_group("mask_tool_settings");
    tool_panel_->set_preference_path("/plugin/grading/tool_panel");

    grading_panel_ = add_string_choice_attribute(
        "grading_panel",
        "grading_panel",
        utility::map_value_to_vec(grading_panel_names_).front(),
        utility::map_value_to_vec(grading_panel_names_));
    grading_panel_->expose_in_ui_attrs_group("grading_settings");
    grading_panel_->set_preference_path("/plugin/grading/grading_panel");

    // Grading elements

    grading_bookmark_ = add_string_attribute("grading_bookmark", "grading_bookmark", "");
    grading_bookmark_->expose_in_ui_attrs_group("grading_settings");

    grade_is_active_ = add_boolean_attribute("grade_active", "grade_active", true);
    grade_is_active_->set_redraw_viewport_on_change(true);
    grade_is_active_->expose_in_ui_attrs_group("grading_settings");

    colour_space_ = add_string_choice_attribute(
        "colour_space", "colour_space", "scene_linear", {"scene_linear", "compositing_log"});
    colour_space_->expose_in_ui_attrs_group("grading_settings");

    working_space_ = add_string_attribute("working_space", "working_space", "");
    working_space_->expose_in_ui_attrs_group("grading_settings");

    grade_in_ = add_integer_attribute("grade_in", "grade_in", -1);
    grade_in_->expose_in_ui_attrs_group("grading_settings");

    grade_out_ = add_integer_attribute("grade_out", "grade_out", -1);
    grade_out_->expose_in_ui_attrs_group("grading_settings");

    // Slope
    slope_ = add_float_vector_attribute(
        "Slope",
        "Slope",
        std::vector<float>({1.0, 1.0, 1.0, 1.0}),                 // initial value
        std::vector<float>({0.0, 0.0, 0.0, std::pow(2.0, -6.0)}), // min
        std::vector<float>({4.0, 4.0, 4.0, std::pow(2.0, 6.0)}),  // max
        std::vector<float>({0.005, 0.005, 0.005, 0.005})          // step
    );
    slope_->expose_in_ui_attrs_group("grading_settings");
    slope_->expose_in_ui_attrs_group("grading_sliders");

    // Offset
    offset_ = add_float_vector_attribute(
        "Offset",
        "Offset",
        std::vector<float>({0.0, 0.0, 0.0, 0.0}),        // initial value
        std::vector<float>({-0.2, -0.2, -0.2, -0.2}),    // min
        std::vector<float>({0.2, 0.2, 0.2, 0.2}),        // max
        std::vector<float>({0.005, 0.005, 0.005, 0.005}) // step
    );
    offset_->expose_in_ui_attrs_group("grading_settings");
    offset_->expose_in_ui_attrs_group("grading_sliders");

    // Power
    power_ = add_float_vector_attribute(
        "Power",
        "Power",
        std::vector<float>({1.0, 1.0, 1.0, 1.0}),        // initial value
        std::vector<float>({0.2, 0.2, 0.2, 0.2}),        // min
        std::vector<float>({4.0, 4.0, 4.0, 4.0}),        // max
        std::vector<float>({0.005, 0.005, 0.005, 0.005}) // step
    );
    power_->expose_in_ui_attrs_group("grading_settings");
    power_->expose_in_ui_attrs_group("grading_sliders");

    // Sat
    sat_ = add_float_attribute("Saturation", "Saturation", 1.0f, 0.0f, 4.0f, 0.005f);
    sat_->set_redraw_viewport_on_change(true);
    sat_->set_role_data(module::Attribute::DefaultValue, 1.0f);
    sat_->expose_in_ui_attrs_group("grading_settings");
    sat_->expose_in_ui_attrs_group("grading_sliders");

    // Masking elements

    drawing_tool_ = add_string_choice_attribute(
        "drawing_tool",
        "drawing_tool",
        utility::map_value_to_vec(drawing_tool_names_).front(),
        utility::map_value_to_vec(drawing_tool_names_));
    drawing_tool_->expose_in_ui_attrs_group("mask_tool_settings");
    drawing_tool_->expose_in_ui_attrs_group("mask_tool_types");

    draw_pen_size_ = add_integer_attribute("Draw Pen Size", "Draw Pen Size", 10, 1, 300);
    draw_pen_size_->expose_in_ui_attrs_group("mask_tool_settings");
    draw_pen_size_->set_preference_path("/plugin/grading/draw_pen_size");

    erase_pen_size_ = add_integer_attribute("Erase Pen Size", "Erase Pen Size", 80, 1, 300);
    erase_pen_size_->expose_in_ui_attrs_group("mask_tool_settings");
    erase_pen_size_->set_preference_path("/plugin/grading/erase_pen_size");

    pen_colour_ = add_colour_attribute(
        "Pen Colour", "Pen Colour", utility::ColourTriplet(0.5f, 0.4f, 1.0f));
    pen_colour_->expose_in_ui_attrs_group("mask_tool_settings");
    pen_colour_->set_preference_path("/plugin/grading/pen_colour");

    pen_opacity_ = add_integer_attribute("Pen Opacity", "Pen Opacity", 100, 0, 100);
    pen_opacity_->expose_in_ui_attrs_group("mask_tool_settings");
    pen_opacity_->set_preference_path("/plugin/grading/pen_opacity");

    pen_softness_ = add_integer_attribute("Pen Softness", "Pen Softness", 0, 0, 100);
    pen_softness_->expose_in_ui_attrs_group("mask_tool_settings");
    pen_softness_->set_preference_path("/plugin/grading/pen_softness");

    display_mode_attribute_ = add_string_choice_attribute(
        "display_mode",
        "display_mode",
        utility::map_value_to_vec(display_mode_names_).front(),
        utility::map_value_to_vec(display_mode_names_));
    display_mode_attribute_->expose_in_ui_attrs_group("mask_tool_settings");
    display_mode_attribute_->set_preference_path("/plugin/grading/display_mode");

    make_behavior();
    listen_to_playhead_events(true);

    // we have to maintain a list of GradingColourOperator instances that are
    // alive to send them messages about our state (currently only the state
    // of the bypass attr)
    set_down_handler([=](down_msg &msg) {
        auto it = grading_colour_op_actors_.begin();
        while (it != grading_colour_op_actors_.end()) {
            if (msg.source == *it) {
                it = grading_colour_op_actors_.erase(it);
            } else {
                it++;
            }
        }
    });

    connect_to_ui();

    // Register the QML code that instances the grading tool UI in an
    // xstudio 'panel'. It will show as 'Grading Tools' under the tabs
    register_ui_panel_qml(
        "Grading Tools",
        R"(
            import QtGraphicalEffects 1.15
            import QtQuick 2.15
            import Grading 1.0
            Item {
                anchors.fill: parent 

                XsGradientRectangle{
                    anchors.fill: parent
                }
                
                GradingDialog {
                    anchors.fill: parent 
                }
            }
        )");
}

utility::BlindDataObjectPtr GradingTool::prepare_overlay_data(
    const media_reader::ImageBufPtr &image, const bool offscreen) const {

    // This callback is made just before viewport redraw. We want to check
    // if the image to be drawn is from the same media to which a grade is
    // currently being edited by us. If so, we attach up-to-date data on
    // the edited grade for display.

    if (!grading_data_.identity() && image) {

        bool we_are_editing_grade_on_this_image = false;
        for (auto &bookmark : image.bookmarks()) {
            if (bookmark->detail_.uuid_ == grading_data_.bookmark_uuid_) {
                we_are_editing_grade_on_this_image = true;
                break;
            }
        }

        if (we_are_editing_grade_on_this_image) {

            auto render_data = std::make_shared<GradingMaskRenderData>();

            // N.B. this means we copy the entirity of grading_data_ (it's strokes
            // basically) on every redraw. Should be ok in the wider scheme of
            // things but not exactly efficient. Another approach would be making
            // GradingData thread safe (Canvas class already is) and share a
            // reference/pointer to grading_data_ here so when drawing happens we're
            // using the interaction member data of this class.
            render_data->interaction_grading_data_ = grading_data_;
            return render_data;
        }
    }
    return utility::BlindDataObjectPtr();
}

AnnotationBasePtr GradingTool::build_annotation(const utility::JsonStore &data) {

    return std::make_shared<GradingData>(data);
}

void GradingTool::images_going_on_screen(
    const std::vector<media_reader::ImageBufPtr> &images,
    const std::string viewport_name,
    const bool playhead_playing) {

    // Only care about the main viewport(s), lightweight viewport will
    // be named like quick_viewport_n.
    if (!utility::starts_with(viewport_name, "viewport")) {
        return;
    }

    // Ignore the callback for a short while after we create or delete
    // a bookmark. This is because the ImageBufPtr info might be out of date.
    // Note that 500ms might be too conservative a value.
    auto time_since_bookmark_update = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - last_bookmark_update_);
    if (time_since_bookmark_update.count() < 500) {
        return;
    }

    current_viewport_ = viewport_name;

    if (grading_tracking_->value()) {

        std::vector<utility::Uuid> on_screen_bookmarks;

        if (images.size()) {

            for (auto &bookmark : images[0].bookmarks()) {

                auto data = dynamic_cast<GradingData *>(bookmark->annotation_.get());
                if (data) {
                    on_screen_bookmarks.push_back(data->bookmark_uuid_);
                }
            }
        }

        utility::Uuid selected_bookmark = utility::Uuid(grading_bookmark_->value());

        auto it = std::find(
            on_screen_bookmarks.begin(), on_screen_bookmarks.end(), selected_bookmark);
        if (it == on_screen_bookmarks.end() && !on_screen_bookmarks.empty()) {
            // Selected bookmark no longer on screen, select the first one available
            select_bookmark(on_screen_bookmarks.front());
        } else if (selected_bookmark && it == on_screen_bookmarks.end()) {
            // Selected bookmark no longer on screen and no bookmarks are currently shown
            // Reset the current bookmark to an empty state
            select_bookmark(utility::Uuid());
        }
    }
}

void GradingTool::on_screen_media_changed(
    caf::actor media_actor,
    const utility::MediaReference &media_ref,
    const std::string media_name,
    const utility::JsonStore &colour_params) {

    const std::string config_name = colour_params.get_or("ocio_config", std::string(""));
    const std::string working_space =
        colour_params.get_or("working_space", std::string("scene_linear"));
    // Only medias that are fully inverted to scene_linear currently support custom colour space
    // This exclude medias that are only inverted to display_linear, for exemple edit_ref.
    const bool is_unmanaged =
        config_name == "" || config_name == "__raw__" || working_space != "scene_linear";

    working_space_->set_value(working_space);
    media_colour_managed_->set_value(!is_unmanaged);
}

void GradingTool::attribute_changed(const utility::Uuid &attribute_uuid, const int role) {

    if (attribute_uuid == tool_is_active_->uuid()) {

        if (tool_is_active_->value()) {
            if (drawing_tool_->value() == "None")
                drawing_tool_->set_value("Draw");
            grab_mouse_focus();

        } else {
            release_mouse_focus();
            release_keyboard_focus();
            end_drawing();
        }

    } else if (attribute_uuid == tool_panel_->uuid()) {

        if (tool_panel_->value() == "Mask") {
            if (drawing_tool_->value() == "None") {
                drawing_tool_->set_value("Draw");
            }
            grab_mouse_focus();

        } else {
            release_mouse_focus();
            release_keyboard_focus();
            end_drawing();
        }

        refresh_current_grade_from_ui();

    } else if (attribute_uuid == grading_bookmark_->uuid()) {

        utility::Uuid bookmark_uuid(grading_bookmark_->value());
        select_bookmark(bookmark_uuid);

    } else if (attribute_uuid == grading_action_->uuid() && grading_action_->value() != "") {

        if (grading_action_->value() == "Clear") {

            clear_cdl();
            refresh_current_grade_from_ui();

        } else if (utility::starts_with(grading_action_->value(), "Save CDL ")) {

            std::size_t prefix_length = std::string("Save CDL ").size();
            std::string filepath      = grading_action_->value().substr(
                prefix_length, grading_action_->value().size() - prefix_length);
            save_cdl(filepath);

        } else if (grading_action_->value() == "Add CC") {

            save_bookmark();
            grading_data_ = GradingData();
            create_bookmark();
            save_bookmark();

        } else if (grading_action_->value() == "Remove CC") {

            remove_bookmark();
        }

        grading_action_->set_value("");

    } else if (attribute_uuid == drawing_action_->uuid() && drawing_action_->value() != "") {

        if (drawing_action_->value() == "Clear") {
            clear_mask();
        } else if (drawing_action_->value() == "Undo") {
            undo();
        } else if (drawing_action_->value() == "Redo") {
            redo();
        }
        drawing_action_->set_value("");

    } else if (attribute_uuid == drawing_tool_->uuid()) {

        if (tool_is_active_->value()) {

            if (drawing_tool_->value() == "None") {
                release_mouse_focus();
            } else {
                grab_mouse_focus();
            }

            end_drawing();
            release_keyboard_focus();
        }

    } else if (attribute_uuid == display_mode_attribute_->uuid()) {

        refresh_current_grade_from_ui();

    } else if (grade_is_active_ && attribute_uuid == grade_is_active_->uuid()) {

        refresh_current_grade_from_ui();
        save_bookmark();

    } else if (grade_in_ && attribute_uuid == grade_in_->uuid()) {

        auto bmd = get_bookmark_detail(current_bookmark());
        if (bmd.media_reference_) {

            auto &media = bmd.media_reference_.value();

            // spdlog::warn("Before Bookmark start: {}", bmd.start_.value().count() /
            // media.rate().to_flicks().count()); spdlog::warn("Before Bookmark duration: {}",
            // bmd.duration_.value().count() / media.rate().to_flicks().count());

            if (grade_in_->value() == -1) {
                grade_in_->set_value(0, false);
            }
            if (grade_out_->value() == -1) {
                grade_out_->set_value(media.frame_count(), false);
            }
            if (grade_in_->value() > grade_out_->value()) {
                grade_out_->set_value(grade_in_->value());
            }

            bmd.start_    = grade_in_->value() * media.rate().to_flicks();
            bmd.duration_ = std::min(
                (grade_out_->value() - grade_in_->value()) * media.rate().to_flicks(),
                media.frame_count() * media.rate().to_flicks());
            update_bookmark_detail(current_bookmark(), bmd);
        }

    } else if (grade_out_ && attribute_uuid == grade_out_->uuid()) {

        auto bmd = get_bookmark_detail(current_bookmark());
        if (bmd.media_reference_) {

            auto &media = bmd.media_reference_.value();

            if (grade_out_->value() == -1) {
                grade_out_->set_value(media.frame_count(), false);
            }
            if (grade_in_->value() == -1) {
                grade_in_->set_value(grade_out_->value(), false);
            }
            if (grade_out_->value() < grade_in_->value()) {
                grade_in_->set_value(grade_out_->value());
            }

            bmd.start_ = grade_in_->value() * media.rate().to_flicks();
            bmd.duration_ =
                (grade_out_->value() - grade_in_->value()) * media.rate().to_flicks();
            update_bookmark_detail(current_bookmark(), bmd);
        }

    } else if (colour_space_ && attribute_uuid == colour_space_->uuid()) {

        refresh_current_grade_from_ui();
        save_bookmark();

    } else if (attribute_uuid == grading_bypass_->uuid()) {

        for (auto &a : grading_colour_op_actors_) {
            send(a, utility::event_atom_v, "bypass", grading_bypass_->value());
        }

    } else if (
        attribute_uuid == slope_->uuid() || attribute_uuid == offset_->uuid() ||
        attribute_uuid == power_->uuid() || attribute_uuid == sat_->uuid()) {

        refresh_current_grade_from_ui();
        create_bookmark_if_empty();
        save_bookmark();
    }

    redraw_viewport();
}

void GradingTool::register_hotkeys() {

    toggle_active_hotkey_ = register_hotkey(
        int('G'),
        ui::ControlModifier,
        "Toggle Grading Tool",
        "Show or hide the grading toolbox");

    toggle_mask_hotkey_ = register_hotkey(
        int('M'),
        ui::NoModifier,
        "Toggle masking",
        "Use drawing tools to apply a matte or apply grading to whole frame");

    undo_hotkey_ = register_hotkey(
        int('Z'),
        ui::ControlModifier,
        "Undo (Annotation edit)",
        "Undoes your last edits to an annotation");

    redo_hotkey_ = register_hotkey(
        int('Z'),
        ui::ControlModifier | ui::ShiftModifier,
        "Redo (Annotation edit)",
        "Redoes your last undone edit on an annotation");
}

void GradingTool::hotkey_pressed(
    const utility::Uuid &hotkey_uuid, const std::string & /*context*/) {

    if (hotkey_uuid == toggle_active_hotkey_) {

        tool_is_active_->set_value(!tool_is_active_->value());

    } else if (hotkey_uuid == toggle_mask_hotkey_ && tool_is_active_->value()) {

        tool_panel_->set_value(tool_panel_->value() == "CC" ? "Mask" : "CC");

    } else if (hotkey_uuid == undo_hotkey_ && tool_is_active_->value()) {

        undo();
        redraw_viewport();

    } else if (hotkey_uuid == redo_hotkey_ && tool_is_active_->value()) {

        redo();
        redraw_viewport();
    }
}

bool GradingTool::pointer_event(const ui::PointerEvent &e) {

    if (!tool_is_active_->value() || !(tool_panel_->value() == "Mask"))
        return false;

    bool redraw = true;

    const Imath::V2f pointer_pos = e.position_in_viewport_coord_sys();

    if (drawing_tool_->value() == "Draw" || drawing_tool_->value() == "Erase") {

        if (e.type() == ui::Signature::EventType::ButtonDown &&
            e.buttons() == ui::Signature::Button::Left) {
            start_stroke(pointer_pos);
        } else if (
            e.type() == ui::Signature::EventType::Drag &&
            e.buttons() == ui::Signature::Button::Left) {
            update_stroke(pointer_pos);
        } else if (e.type() == ui::Signature::EventType::ButtonRelease) {
            end_drawing();
        }
    } else {
        redraw = false;
    }

    if (redraw)
        redraw_viewport();

    return false;
}

void GradingTool::start_stroke(const Imath::V2f &point) {

    if (drawing_tool_->value() == "Draw") {
        grading_data_.mask().start_stroke(
            pen_colour_->value(),
            draw_pen_size_->value() / PEN_STROKE_THICKNESS_SCALE,
            pen_softness_->value() / 100.0,
            pen_opacity_->value() / 100.0);
    } else if (drawing_tool_->value() == "Erase") {
        grading_data_.mask().start_erase_stroke(
            erase_pen_size_->value() / PEN_STROKE_THICKNESS_SCALE);
    }

    update_stroke(point);

    create_bookmark_if_empty();
}

void GradingTool::update_stroke(const Imath::V2f &point) {

    grading_data_.mask().update_stroke(point);
}

void GradingTool::end_drawing() {

    grading_data_.mask().end_draw();
    save_bookmark();
}

void GradingTool::undo() {

    if (tool_panel_->value() == "Mask") {

        grading_data_.mask().undo();
    }
    save_bookmark();
}

void GradingTool::redo() {

    if (tool_panel_->value() == "Mask") {

        grading_data_.mask().redo();
    }
    save_bookmark();
}

void GradingTool::clear_mask() {

    grading_data_.mask().clear();
    save_bookmark();
}

void GradingTool::clear_cdl() {

    slope_->set_value(
        slope_->get_role_data<std::vector<float>>(module::Attribute::DefaultValue));
    offset_->set_value(
        offset_->get_role_data<std::vector<float>>(module::Attribute::DefaultValue));
    power_->set_value(
        power_->get_role_data<std::vector<float>>(module::Attribute::DefaultValue));
    sat_->set_value(sat_->get_role_data<float>(module::Attribute::DefaultValue));
}

void GradingTool::save_cdl(const std::string &filepath) const {

    OCIO::CDLTransformRcPtr cdl = OCIO::CDLTransform::Create();

    std::array<double, 3> slope{
        slope_->value()[0] * slope_->value()[3],
        slope_->value()[1] * slope_->value()[3],
        slope_->value()[2] * slope_->value()[3]};

    std::array<double, 3> offset{
        offset_->value()[0] + offset_->value()[3],
        offset_->value()[1] + offset_->value()[3],
        offset_->value()[2] + offset_->value()[3]};

    std::array<double, 3> power{
        power_->value()[0] * power_->value()[3],
        power_->value()[1] * power_->value()[3],
        power_->value()[2] * power_->value()[3]};

    cdl->setSlope(slope.data());
    cdl->setOffset(offset.data());
    cdl->setPower(power.data());
    cdl->setSat(sat_->value());

    OCIO::FormatMetadata &metadata = cdl->getFormatMetadata();
    metadata.setID("0");

    OCIO::GroupTransformRcPtr grp = OCIO::GroupTransform::Create();
    grp->appendTransform(cdl);

    // Write to disk using OCIO

    std::string localpath = filepath;
    localpath             = utility::replace_once(localpath, "file://", "");

    std::string format;
    if (utility::ends_with(localpath, "cdl")) {
        format = "ColorDecisionList";
    } else if (utility::ends_with(localpath, "cc")) {
        format = "ColorCorrection";
    } else if (utility::ends_with(localpath, "ccc")) {
        format = "ColorCorrectionCollection";
    }

    std::ofstream ofs(localpath);
    if (ofs.is_open()) {
        grp->write(OCIO::GetCurrentConfig(), format.c_str(), ofs);
    } else {
        spdlog::warn("Failed to create file: {}", localpath);
    }
}

void GradingTool::refresh_current_grade_from_ui() {

    auto &grade = grading_data_.grade();

    grade.slope  = vector4f_to_array4d(slope_->value());
    grade.offset = vector4f_to_array4d(offset_->value());
    grade.power  = vector4f_to_array4d(power_->value());
    grade.sat    = sat_->value();

    grading_data_.set_grade_active(grade_is_active_->value());
    grading_data_.set_colour_space(colour_space_->value());
    grading_data_.set_mask_editing(display_mode_attribute_->value() == "Mask");
}

void GradingTool::refresh_ui_from_current_grade() {

    auto &grade = grading_data_.grade();

    slope_->set_value(array4d_to_vector4f(grade.slope), false);
    offset_->set_value(array4d_to_vector4f(grade.offset), false);
    power_->set_value(array4d_to_vector4f(grade.power), false);
    sat_->set_value(float(grade.sat), false);

    grade_is_active_->set_value(grading_data_.grade_active(), false);
    colour_space_->set_value(grading_data_.colour_space(), false);
    display_mode_attribute_->set_value(grading_data_.mask_editing() ? "Mask" : "Grade", false);
}

utility::Uuid GradingTool::current_bookmark() const {

    return utility::Uuid(grading_bookmark_->value());
}

void GradingTool::create_bookmark_if_empty() {

    if (!current_bookmark()) {
        create_bookmark();
    }
}

void GradingTool::create_bookmark() {

    bookmark::BookmarkDetail bmd;
    // Hides bookmark from timeline
    bmd.colour_  = "transparent";
    bmd.visible_ = false;

    auto uuid = StandardPlugin::create_bookmark_on_current_media(
        "",             // viewport_name
        "Grading Note", // bookmark_subject
        bmd,            // detail
        true            // bookmark_entire_duration
    );

    grading_data_.bookmark_uuid_ = uuid;
    grading_data_.set_colour_space(working_space_->value());
    grading_bookmark_->set_value(utility::to_string(uuid), false);

    refresh_ui_from_current_grade();

    last_bookmark_update_ = std::chrono::high_resolution_clock::now();

    // spdlog::warn("Created bookmark {}", utility::to_string(grading_data_.bookmark_uuid_));
}

void GradingTool::select_bookmark(const utility::Uuid &uuid) {

    // spdlog::warn("Select bookmark {}", utility::to_string(uuid));

    GradingData *grading_data_ptr = nullptr;
    if (uuid) {
        auto base_ptr    = get_bookmark_annotation(uuid);
        grading_data_ptr = dynamic_cast<GradingData *>(base_ptr.get());
    }

    if (grading_data_ptr) {
        grading_data_ = *grading_data_ptr;
    } else {
        grading_data_ = GradingData();
        grading_data_.set_colour_space(working_space_->value());
    }

    grading_data_.bookmark_uuid_ = uuid;
    grading_bookmark_->set_value(utility::to_string(uuid), false);

    refresh_ui_from_current_grade();
}

void GradingTool::save_bookmark() {

    if (current_bookmark()) {

        StandardPlugin::update_bookmark_annotation(
            current_bookmark(), std::make_shared<GradingData>(grading_data_), false);
        // spdlog::warn("Saved bookmark {}", utility::to_string(current_bookmark()));
    }
}

void GradingTool::remove_bookmark() {

    if (current_bookmark()) {

        // spdlog::warn("Removing bookmark {}", utility::to_string(current_bookmark()));
        StandardPlugin::remove_bookmark(current_bookmark());

        last_bookmark_update_ = std::chrono::high_resolution_clock::now();
    }

    const auto &bookmarks_list = get_bookmarks_on_current_media(current_viewport_);
    if (!bookmarks_list.empty()) {
        select_bookmark(bookmarks_list.back());
    } else {
        select_bookmark(utility::Uuid());
    }
}

caf::message_handler GradingTool::message_handler_extensions() {
    return caf::message_handler({[=](const std::string &desc, caf::actor grading_colour_op) {
               if (desc == "follow_bypass") {
                   grading_colour_op_actors_.push_back(grading_colour_op);
                   monitor(grading_colour_op);
                   send(
                       grading_colour_op,
                       utility::event_atom_v,
                       "bypass",
                       grading_bypass_->value());
               }
           }})
        .or_else(StandardPlugin::message_handler_extensions());
}


static std::vector<std::shared_ptr<plugin_manager::PluginFactory>> factories(
    {std::make_shared<plugin_manager::PluginFactoryTemplate<GradingTool>>(
         GradingTool::PLUGIN_UUID,
         "GradingToolUI",
         plugin_manager::PluginFlags::PF_VIEWPORT_OVERLAY,
         true,
         "Remi Achard",
         "Plugin providing interface for creating interactive grading notes with painted "
         "masks.",
         semver::version("0.0.0"),
         "",
         ""),
     std::make_shared<plugin_manager::PluginFactoryTemplate<GradingColourOperator>>(
         GradingColourOperator::PLUGIN_UUID,
         "GradingToolColourOp",
         plugin_manager::PluginFlags::PF_COLOUR_OPERATION,
         false,
         "Remi Achard",
         "Colour operator to apply CDL with optional painted masking in viewport.")});

#define PLUGIN_DECLARE_END()                                                                   \
    extern "C" {                                                                               \
    plugin_manager::PluginFactoryCollection *plugin_factory_collection_ptr() {                 \
        return new plugin_manager::PluginFactoryCollection(                                    \
            std::vector<std::shared_ptr<plugin_manager::PluginFactory>>(factories));           \
    }                                                                                          \
    }

PLUGIN_DECLARE_END()