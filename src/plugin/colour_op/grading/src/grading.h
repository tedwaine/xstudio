// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <chrono>

#include <OpenColorIO/OpenColorIO.h> //NOLINT

#include "xstudio/colour_pipeline/colour_operation.hpp"
#include "grading_data.h"
#include "grading_mask_gl_renderer.h"

namespace OCIO = OCIO_NAMESPACE;


namespace xstudio::colour_pipeline {

class GradingTool : public plugin::StandardPlugin {
    public:
        inline static const utility::Uuid PLUGIN_UUID =
            utility::Uuid("5598e01e-c6bc-4cf9-80ff-74bb560df12a");

  public:
    GradingTool(caf::actor_config &cfg, const utility::JsonStore &init_settings);
    ~GradingTool() override = default;

    utility::BlindDataObjectPtr prepare_overlay_data(
        const media_reader::ImageBufPtr &, const bool offscreen) const override;

    // Annotations (grading)

    bookmark::AnnotationBasePtr build_annotation(const utility::JsonStore &data) override;

    void images_going_on_screen(
        const std::vector<media_reader::ImageBufPtr> & images,
        const std::string viewport_name,
        const bool playhead_playing
    ) override;

    void on_screen_media_changed(
        caf::actor,
        const utility::MediaReference &,
        const std::string,
        const utility::JsonStore &
    ) override;

    // Interactions

    void attribute_changed(
        const utility::Uuid &attribute_uuid, const int role) override;

    void register_hotkeys() override;
    void hotkey_pressed(const utility::Uuid &hotkey_uuid, const std::string &context) override;

    bool pointer_event(const ui::PointerEvent &e) override;

  protected:
      caf::message_handler message_handler_extensions() override;

  private:
    void start_stroke(const Imath::V2f &point);
    void update_stroke(const Imath::V2f &point);

    void start_quad(const std::vector<Imath::V2f> &corners);
    void start_polygon(const std::vector<Imath::V2f> &points);
    void start_ellipse(const Imath::V2f &center, const Imath::V2f &radius, float angle);

    void remove_shape(uint32_t id);

    void end_drawing();

    void undo();
    void redo();

    void clear_mask();
    void clear_shapes();
    void clear_cdl();
    void save_cdl(const std::string &filepath) const;

    utility::Uuid current_bookmark() const;
    void create_bookmark_if_empty();
    void create_bookmark();
    void select_bookmark(const utility::Uuid &uuid);
    void save_bookmark();
    void remove_bookmark();

    void refresh_current_grade_from_ui();
    void refresh_ui_from_current_grade();

  private:
    // General
    module::BooleanAttribute *tool_is_active_       {nullptr};
    module::StringAttribute  *grading_action_       {nullptr};
    module::BooleanAttribute *grading_bypass_       {nullptr};
    module::StringAttribute  *drawing_action_       {nullptr};
    module::BooleanAttribute *grading_tracking_     {nullptr};
    module::BooleanAttribute *media_colour_managed_ {nullptr};

    enum class ToolPanel { CC, Mask };
    const std::map<ToolPanel, std::string> tool_panel_names_ = {
        {ToolPanel::CC, "CC"},
        {ToolPanel::Mask, "Mask"}
    };
    module::StringChoiceAttribute *tool_panel_ {nullptr};

    enum class GradingPanel { Basic, CDLSliders, CDLWheels };
    const std::map<GradingPanel, std::string> grading_panel_names_ = {
        {GradingPanel::Basic, "Basic"},
        {GradingPanel::CDLSliders, "Sliders"},
        {GradingPanel::CDLWheels, "Wheels"}
    };
    module::StringChoiceAttribute *grading_panel_    {nullptr};

    // Grading
    module::StringAttribute       *grading_bookmark_ {nullptr};
    module::BooleanAttribute      *grade_is_active_  {nullptr};
    module::StringAttribute       *working_space_    {nullptr};
    module::StringChoiceAttribute *colour_space_     {nullptr};
    module::IntegerAttribute      *grade_in_         {nullptr};
    module::IntegerAttribute      *grade_out_        {nullptr};

    module::FloatVectorAttribute  *slope_  {nullptr};
    module::FloatVectorAttribute  *offset_ {nullptr};
    module::FloatVectorAttribute  *power_  {nullptr};
    module::FloatAttribute        *sat_    {nullptr};

    // Drawing Mask
    enum class DrawingTool { Draw, Erase, Shape, None };
    const std::map<DrawingTool, std::string> drawing_tool_names_ = {
        {DrawingTool::Draw, "Draw"},
        {DrawingTool::Erase, "Erase"},
        {DrawingTool::Shape, "Shape"}
    };

    module::StringChoiceAttribute *drawing_tool_ {nullptr};
    module::IntegerAttribute      *draw_pen_size_     {nullptr};
    module::IntegerAttribute      *erase_pen_size_    {nullptr};
    module::IntegerAttribute      *pen_opacity_       {nullptr};
    module::IntegerAttribute      *pen_softness_      {nullptr};
    module::ColourAttribute       *pen_colour_        {nullptr};
    module::BooleanAttribute      *shape_invert_      {nullptr};
    module::BooleanAttribute      *polygon_init_      {nullptr};

    module::IntegerAttribute *mask_selected_shape_{nullptr};
    std::vector<module::Attribute *> mask_shapes_;

    enum DisplayMode { Mask, Grade };
    const std::map<DisplayMode, std::string> display_mode_names_ = {
        {DisplayMode::Grade, "Grade"},
        {DisplayMode::Mask, "Mask"}
    };
    module::StringChoiceAttribute *display_mode_attribute_  {nullptr};

    // Shortcuts
    utility::Uuid toggle_active_hotkey_;
    utility::Uuid toggle_mask_hotkey_;
    utility::Uuid undo_hotkey_;
    utility::Uuid redo_hotkey_;

    // Current media info (eg. for Bookmark creation)
    bool playhead_is_playing_ {false};

    std::string current_viewport_;

    // Grading
    ui::viewport::GradingData grading_data_;

    std::chrono::time_point<std::chrono::high_resolution_clock> last_bookmark_update_;

    std::vector<caf::actor> grading_colour_op_actors_;
};

} // xstudio::colour_pipeline
