// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cmath>
#include <cfloat>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeinfo>

#include <OpenColorIO/OpenColorIO.h> //NOLINT

#include "xstudio/colour_pipeline/colour_pipeline_actor.hpp"
#include "xstudio/plugin_manager/plugin_manager.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/global_store/global_store.hpp"
#include "xstudio/thumbnail/thumbnail.hpp"
#include "ui_text.hpp"
#include "ocio_shared_settings.hpp"
#include "ocio_engine.hpp"

namespace OCIO = OCIO_NAMESPACE;

namespace xstudio::colour_pipeline::ocio {

class OCIOColourPipeline : public ColourPipeline, private OCIOEngine {


  public:
    explicit OCIOColourPipeline(
        caf::actor_config &cfg, const utility::JsonStore &init_settings = utility::JsonStore());

    virtual ~OCIOColourPipeline() override;

    std::string fast_display_transform_hash(const media::AVFrameID &media_ptr) override;

    /* Create the ColourOperationDataPtr containing the necessary LUT and
    shader data for linearising the source colourspace RGB data from the
    given media source on the screen */
    void linearise_op_data(
        caf::typed_response_promise<ColourOperationDataPtr> &rp,
        const media::AVFrameID &media_ptr) override;

    /* Create the ColourOperationDataPtr containing the necessary LUT and
    shader data for transforming linear colour values into display space */
    void linear_to_display_op_data(
        caf::typed_response_promise<ColourOperationDataPtr> &rp,
        const media::AVFrameID &media_ptr) override;

    // Update colour pipeline shader dynamic parameters.
    utility::JsonStore update_shader_uniforms(
        const media_reader::ImageBufPtr &image, std::any &user_data) override;

    void process_thumbnail(
        caf::typed_response_promise<thumbnail::ThumbnailBufferPtr> &rp,
        const media::AVFrameID &media_ptr,
        const thumbnail::ThumbnailBufferPtr &buf) override;

    // GUI handling
    void media_source_changed(
        const utility::Uuid &source_uuid,
        const utility::JsonStore &src_colour_mgmt_metadata) override;
    void attribute_changed(const utility::Uuid &attribute_uuid, const int /*role*/) override;

    void screen_changed(
        const std::string &name,
        const std::string &model,
        const std::string &manufacturer,
        const std::string &serialNumber) override;

    void connect_to_viewport(
        const std::string &viewport_name,
        const std::string &viewport_toolbar_name,
        bool connect) override;

    void extend_pixel_info(
        media_reader::PixelInfo &pixel_info, const media::AVFrameID &frame_id) override;

    caf::message_handler message_handler_extensions() override;

  private:
    std::string display() const;

    std::string view(const media::AVFrameID &media_ptr) const;

    void setup_ui();

    void populate_ui(const utility::JsonStore &src_colour_mgmt_metadata);

    void update_views(const std::string new_display);

    void update_bypass(bool bypass);

  private:
    // GUI handling
    UiText ui_text_;

    caf::actor global_controls_;

    caf::actor worker_pool_;

    module::StringChoiceAttribute *display_;
    module::StringChoiceAttribute *view_;

    struct OCIOControlsData {
        bool colour_bypass_      = {false};
        bool use_preferred_view_ = {true};
        std::string preferred_view_;
        float exposure_      = {0.0f};
        float gamma_         = {1.0f};
        float saturation_    = {1.0f};
        std::string channel_ = {"RGB"};
    } settings_;

    // Holds info about the currently on screen media
    utility::Uuid current_source_uuid_;
    std::string current_config_name_;
    std::string last_update_hash_;
    utility::JsonStore current_source_colour_mgmt_metadata_;

    // Holds data on display screen option
    std::string monitor_name_;
    std::string viewport_name_;

    std::vector<std::string> all_colourspaces_;
    std::vector<std::string> displays_;
    std::map<std::string, std::vector<std::string>> display_views_;
};

} // namespace xstudio::colour_pipeline::ocio
