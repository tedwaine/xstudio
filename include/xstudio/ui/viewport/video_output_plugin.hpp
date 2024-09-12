// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/audio/audio_output_actor.hpp"
#include "xstudio/plugin_manager/plugin_base.hpp"

namespace xstudio {
namespace ui {
    namespace viewport {

    class VideoOutputPlugin : public plugin::StandardPlugin {

        public:

        // Note - when deriving from VideoOutputPlugin your constructor must have
        // a signature of this form:
        //
        // MyVidOutputPlugin(caf::actor_config &cfg, const utility::JsonStore &init_settings)
        //
        // cgf and init_settings must be supplied to base class with a unique plugin name

        VideoOutputPlugin(caf::actor_config &cfg, const utility::JsonStore &init_settings, const std::string &plugin_name);
        ~VideoOutputPlugin() override = default;

        // This method should be implemented to allow cleanup of any/all resources
        // relating to the video output
        virtual void exit_cleanup() = 0;

        // This method is called when a new image buffer is ready to be displayed
        virtual void incoming_video_frame_callback(media_reader::ImageBufPtr incoming) = 0;

        // Allocate your resources needed for video output, initialise hardware etc. in
        // this function
        virtual void initialise() = 0;

        void on_exit() override;

        // Call this method to intiate rendering of the xstudio viewport to an offscreen
        // surface. The resulting pixel buffers are captured and returned via the
        // 'incoming_video_frame_callback' method
        //
        // This method can be safely called from any thread
        void start(int frame_width, int frame_height);

        // Call this method to stop rendering and cease calls to the 
        // incoming_video_frame_callback method
        //
        // This method can be safely called from any thread
        void stop();

        // This method MUST be called on every refresh of the video output to tell the
        // offscreen viewport to send a new video frame. The 'beat' of frame_display_time
        // is important to tell the offscreen viewport the natural refresh rate of the
        // video output device and to sync the playhead to your video output.
        //
        // frame_display_time should inform as accurately as possible when the last frame 
        // delivered via incoming_video_frame_callback is actually shown on the display.
        //
        // This method can be safely called from any thread
        void frame_consumed(const utility::time_point & frame_display_time = utility::clock::now());

        // Override this method to instance a class derived from AudioOutputDevice with new
        // that can receive an audio sample stream from xSTUDIO and deliver to your physical/virtual
        // output device. Ownership of this object (and it's destruction) is with xSTUDIO.
        virtual audio::AudioOutputDevice * make_audio_output_device(const utility::JsonStore &prefs) { return nullptr; }

        // This method can be safely called from any thread
        void set_status(const std::string & status_message);

        // This method can be safely called from any thread
        void set_error(const std::string & error_message);

      private:

        void spawn_audio_output_actor(const utility::JsonStore &prefs) {
            auto audio_dev = make_audio_output_device(prefs);
            if (audio_dev) {
                auto a = spawn<audio::AudioOutputActor>(std::shared_ptr<audio::AudioOutputDevice>(audio_dev));
                link_to(a);
            }
        }


        caf::message_handler message_handler_extensions() override {
            return message_handler_extensions_;
        }

        caf::actor offscreen_viewport_;
        caf::actor main_viewport_;
        caf::message_handler message_handler_extensions_;

    };
} // namespace viewport
} // namespace ui
} // namespace xstudio
