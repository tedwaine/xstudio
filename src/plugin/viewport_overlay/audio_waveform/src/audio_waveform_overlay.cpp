// SPDX-License-Identifier: Apache-2.0
#include <caf/actor_registry.hpp>

#include "audio_waveform_overlay.hpp"
#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/media_reader/image_buffer.hpp"
#include "xstudio/global_store/global_store.hpp"
#include "xstudio/utility/blind_data.hpp"
#include "xstudio/ui/viewport/viewport_helpers.hpp"
#include "xstudio/utility/helpers.hpp"

#include "audio_waveform_overlay_renderer.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;

AudioWaveformOverlay::AudioWaveformOverlay(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : plugin::HUDPluginBase(cfg, "Audio Waveform", init_settings, 0.0f) {

    add_hud_description(
        "This overlay draws the sound waveform corresponding to the current on-screen frame. "
        "You can see more or less of the audio waveform by varying the 'Horizontal Scale' "
        "value. If 'Horizontal Scale' is set to 1.0 you will see the waveform corresponding "
        "exactly to the sound you hear when frame scrubbing / stepping (if you have audio "
        "scrubbing enabled in your preferences). The red line indicates the central point of "
        "the audio wave for the current frame. The audio samples that fall outside of the "
        "audio scrub window (i.e. the samples you won't hear for this frame) are drawn with a "
        "faded line.");

    horizontal_scale_ =
        add_float_attribute("Horizontal Scale", "Horizontal Scale", 1.0f, 1.0f, 5.0f, 0.1f);
    add_hud_settings_attribute(horizontal_scale_);
    horizontal_scale_->set_tool_tip(
        "Sets the horizontal scaling of the waveform - the unit "
        "corresponds to the number of frames of audio shown on the screen.");
    vertical_scale_ =
        add_float_attribute("Vertical Scale", "Vertical Scale", 0.1f, 0.01f, 1.0f, 0.01f);
    add_hud_settings_attribute(vertical_scale_);
    vertical_scale_->set_tool_tip("Sets the vertical scaling of the waveform");

    chan_position_spacing_ = add_float_attribute(
        "Chan Position Spacing", "Chan Position Spacing", 0.05f, 0.0f, 1.0f, 0.01f);
    add_hud_settings_attribute(chan_position_spacing_);
    chan_position_spacing_->set_tool_tip("Vertical spacing between channels");

    vertical_position_ = add_float_attribute(
        "Vertical Position", "Vertical Position", -0.8f, -1.0f, 1.0f, 0.01f);
    add_hud_settings_attribute(vertical_position_);
    vertical_position_->set_tool_tip("Vertical position for drawing the waveform");

    separate_channels_ =
        add_boolean_attribute("Show Channels Separately", "Show Channels Separately", false);
    add_hud_settings_attribute(separate_channels_);
    separate_channels_->set_tool_tip(
        "Shows the waveforms of each channel, or combine channels if not selected.");

    in_frame_waveform_colour_ = add_colour_attribute(
        "Inside Frame Colour", "In Frame Colour", utility::ColourTriplet(1.0f, 1.0f, 0.0f));
    add_hud_settings_attribute(in_frame_waveform_colour_);
    in_frame_waveform_colour_->set_tool_tip("The colour of the waveform line");

    outside_frame_waveform_colour_ = add_colour_attribute(
        "Outside Frame Colour",
        "Outside Frame Colour",
        utility::ColourTriplet(0.4f, 0.4f, 1.0f));
    add_hud_settings_attribute(outside_frame_waveform_colour_);
    outside_frame_waveform_colour_->set_tool_tip("The colour of the waveform line");

    // Registering preference path allows these values to persist between sessions
    vertical_scale_->set_preference_path("/plugin/audio_waveform/vertical_scale");
    horizontal_scale_->set_preference_path("/plugin/audio_waveform/horizontal_scale_frames");
    chan_position_spacing_->set_preference_path("/plugin/audio_waveform/chan_position_spacing");
    vertical_position_->set_preference_path("/plugin/audio_waveform/vertical_position");
    in_frame_waveform_colour_->set_preference_path("/plugin/audio_waveform/line_colour");
    outside_frame_waveform_colour_->set_preference_path(
        "/plugin/audio_waveform/extra_line_colour");

    // get the global audio output actor and join its event group. This means we
    // receive the broadcasted Audiobuffers
    auto global_audio_actor =
        system().registry().template get<caf::actor>(audio_output_registry);
    utility::join_event_group(this, global_audio_actor);

    // this kicks the global_audio_actor to send us the scrub settings (first
    // message handler below)
    anon_mail(module::change_attribute_event_atom_v, caf::actor_cast<caf::actor>(this))
        .send(global_audio_actor);

    message_handler_ext_ = {
        [=](utility::event_atom,
            module::change_attribute_event_atom,
            const float volume,
            const bool muted,
            const bool repitch,
            const bool scrubbing,
            const std::string scrub_behaviour,
            const int scrub_window_millisecs) {
            scrub_helper_.set_behaviour(scrub_behaviour);
            scrub_helper_.set_custom_duration_ms(scrub_window_millisecs);
            redraw_viewport();
        },
        [=](utility::event_atom,
            playhead::sound_audio_atom,
            const std::vector<media_reader::AudioBufPtr> &audio_buffers,
            const utility::Uuid &sub_playhead,
            const bool scrubbing,
            const timebase::flicks,
            const float playhead_vol) {},
        [=](utility::event_atom,
            playhead::position_atom,
            const timebase::flicks playhead_position,
            const timebase::flicks in,
            const timebase::flicks out,
            const bool forward,
            const float velocity,
            const bool playing,
            utility::time_point when_position_changed) {},
        [=](utility::event_atom,
            playhead::position_atom,
            const timebase::flicks playhead_position,
            const timebase::flicks in,
            const timebase::flicks out,
            const bool forward,
            const float velocity,
            const bool playing,
            utility::time_point when_position_changed) {},
        [=](utility::event_atom,
            audio::audio_samples_atom,
            const std::vector<media_reader::AudioBufPtr> &audio_buffers,
            timebase::flicks playhead_position,
            const utility::Uuid &playhead_uuid) {
            latest_audio_buffers_[playhead_uuid] = audio_buffers;
        }};

    make_behavior();
    // we need to keep track of which playhead is driving which viewport
    listen_to_playhead_events();
}

AudioWaveformOverlay::~AudioWaveformOverlay() = default;

void AudioWaveformOverlay::attribute_changed(
    const utility::Uuid &attribute_uuid, const int /*role*/
) {

    redraw_viewport();
}

utility::BlindDataObjectPtr AudioWaveformOverlay::onscreen_render_data(
    const media_reader::ImageBufPtr &image,
    const std::string & /*viewport_name*/,
    const utility::Uuid &playhead_uuid,
    const bool is_hero_image,
    const bool images_are_in_grid_layout) const {

    auto r = utility::BlindDataObjectPtr();
    if (!visible())
        return r;

    auto p = latest_audio_buffers_.find(playhead_uuid);
    if (p == latest_audio_buffers_.end())
        return r;
    const auto &latest_audio_buffers = p->second;

    // check our sample buffers to get sample rate & num channels
    int nc                  = 0;
    uint64_t sample_rate    = 0;
    double aud_buf_duration = 0.0;
    for (const auto &aud_buf : latest_audio_buffers) {

        if (aud_buf && !sample_rate && !nc) {
            nc               = aud_buf->num_channels();
            sample_rate      = aud_buf->sample_rate();
            aud_buf_duration = aud_buf->duration_seconds();
        }
    }

    if (!sample_rate)
        return r;

    // the number of samples we need depends on the audio scurbbing duration and
    // horizontal_scale_
    const auto window = scrub_helper_.scrub_duration(aud_buf_duration) *
                        int(horizontal_scale_->value() * 10000) / 10000;

    const int samps_needed = int(round(timebase::to_seconds(window) * double(sample_rate)));

    std::vector<float> verts(samps_needed * (separate_channels_->value() ? nc : 1));

    // this gives us the ref timestamp for the start of the window of samples that
    // we will draw to the screen
    timebase::flicks tt = image.timeline_timestamp() - (window - image.frame_id().rate()) / 2;

    for (const auto &aud_buf : latest_audio_buffers) {

        if (aud_buf) {
            // reference timeline timestamp for first sample
            timebase::flicks when_samples_play = aud_buf.timeline_timestamp();

            const int nsamp = aud_buf->num_samples();

            if (separate_channels_->value()) {

                // offset *into* the samples that we're generating
                for (int c = 0; c < nc; ++c) {
                    int offset = timebase::to_seconds(when_samples_play - tt) * sample_rate;
                    int n      = 0;
                    if (offset < 0) {
                        n      = -offset;
                        offset = 0;
                    }
                    int16_t *samp_data = (int16_t *)aud_buf->buffer();
                    samp_data += n * nc + c;
                    while (offset < samps_needed && n < nsamp) {

                        verts[offset + samps_needed * c] = float(*samp_data) * 0.000030518f;
                        n++;
                        offset++;
                        samp_data += nc;
                    }
                }

            } else {

                int offset = timebase::to_seconds(when_samples_play - tt) * sample_rate;
                int n      = 0;
                if (offset < 0) {
                    n      = -offset;
                    offset = 0;
                }
                int16_t *samp_data = (int16_t *)aud_buf->buffer();
                samp_data += n * nc;
                while (offset < samps_needed && n < nsamp) {

                    float f = 0.0f;
                    int c   = nc;
                    while (c--) {
                        f += *(samp_data++);
                    }
                    verts[offset] = f * 0.000030518f;
                    n++;
                    offset++;
                }
            }
        }
    }

    r.reset(new WaveFormData(
        verts,
        separate_channels_->value() ? nc : 1,
        vertical_scale_->value(),
        chan_position_spacing_->value(),
        vertical_position_->value(),
        in_frame_waveform_colour_->value(),
        outside_frame_waveform_colour_->value(),
        horizontal_scale_->value()));

    return r;
}

plugin::ViewportOverlayRendererPtr
AudioWaveformOverlay::make_overlay_renderer(const std::string &viewport_name) {
    return plugin::ViewportOverlayRendererPtr(new AudioWaveformOverlayRenderer());
}

/*void AudioWaveformOverlay::viewport_playhead_changed(const std::string &viewport_name,
caf::actor playhead) { if (playhead) { request(playhead, infinite, utility::uuid_atom_v).then(
            [=](utility::Uuid &playhead_uuid) {
            },
            [=](caf::error &err) {});
    }
}*/

extern "C" {
plugin_manager::PluginFactoryCollection *plugin_factory_collection_ptr() {
    return new plugin_manager::PluginFactoryCollection(
        std::vector<std::shared_ptr<plugin_manager::PluginFactory>>(
            {std::make_shared<plugin_manager::PluginFactoryTemplate<AudioWaveformOverlay>>(
                utility::Uuid("873c508b-276b-44e3-82d0-15db2f039aa7"),
                "AudioWaveformOverlay",
                plugin_manager::PluginFlags::PF_HEAD_UP_DISPLAY |
                    plugin_manager::PluginFlags::PF_VIEWPORT_OVERLAY,
                true,
                "Ted Waine",
                "Audio Waveform Overlay")}));
}
}
