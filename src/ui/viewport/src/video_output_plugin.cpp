// SPDX-License-Identifier: Apache-2.0
#include "xstudio/ui/viewport/video_output_plugin.hpp"

using namespace xstudio::ui::viewport;
using namespace xstudio::plugin;

VideoOutputPlugin::VideoOutputPlugin(
    caf::actor_config &cfg, const utility::JsonStore &init_settings, const std::string &plugin_name)
    : xstudio::plugin::StandardPlugin(cfg, plugin_name, init_settings) 
{

     // provide an extension to the base class message handler to handle timed
    // callbacks to fade the laser pen strokes
    message_handler_extensions_ = {
        [=](offscreen_viewport_atom, caf::actor offscreen_vp) {

        },
        [=](media_reader::ImageBufPtr incoming) {
            incoming_video_frame_callback(incoming);
        },
        [=](const std::string &status_msg, bool is_error) {
        },
        [=](caf::error & err) {
        }};

    // this call is essential to set-up the base class
    make_behavior();

    // this ensures 'Attributes' created by derived class get exposed in the UI layer
    connect_to_ui();

    // Get the 'StudioUI' which lives in the Qt context and therefore is able to
    // create offscreen viewports for us
    auto studio_ui = system().registry().template get<caf::actor>(studio_ui_registry);

    // tell the studio actor to create an offscreen viewport. It will send
    // us the resulting actor asynchronously as a message which our message
    // handler above will receive
    request(studio_ui, infinite, offscreen_viewport_atom_v, Module::name() + " viewport").then(
        [=](caf::actor offscreen_vp) {

            // this is the offscreen renderer that we asked for below.
            offscreen_viewport_ = offscreen_vp;
            
            // now we have an offscreen viewport to send us frame buffers
            // we can initialise the card and start output
            initialise();

            spawn_audio_output_actor(init_settings);

        },
        [=](caf::error & err) mutable {
            spdlog::critical("{} in plugin {} : {}", __PRETTY_FUNCTION__, Module::name(), to_string(err));
        });

}

void VideoOutputPlugin::on_exit() {
    exit_cleanup();
    StandardPlugin::on_exit();
}

void VideoOutputPlugin::start(int frame_width, int frame_height) {

    if (!offscreen_viewport_) return;

    send(offscreen_viewport_,
        video_output_actor_atom_v,
        caf::actor_cast<caf::actor>(this),
        frame_width, 
        frame_height,
        viewport::RGBA_16 // viewport::RGBA_10_10_10_2 // *see note below
        );

}

void VideoOutputPlugin::stop() {

    send(
        offscreen_viewport_,
        video_output_actor_atom_v,
        caf::actor()
        );

}

void VideoOutputPlugin::frame_consumed(const utility::time_point & frame_display_time) {
    anon_send(offscreen_viewport_, ui::fps_monitor::framebuffer_swapped_atom_v, frame_display_time);
    
    // we also tell the viewport to immediately render the next frame
    anon_send(offscreen_viewport_, ui::viewport::render_viewport_to_image_atom_v);

}

void VideoOutputPlugin::set_status(const std::string & status_message) {

}

void VideoOutputPlugin::set_error(const std::string & error_message) {

}
