// SPDX-License-Identifier: Apache-2.0
#include <caf/policy/select_all.hpp>

#include "xstudio/atoms.hpp"
#include "xstudio/broadcast/broadcast_actor.hpp"
#include "xstudio/playhead/playhead_global_events_actor.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/uuid.hpp"

using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::playhead;
using namespace xstudio::media;
using namespace caf;


PlayheadGlobalEventsActor::PlayheadGlobalEventsActor(caf::actor_config &cfg)
    : caf::event_based_actor(cfg) {

    init();
}

void PlayheadGlobalEventsActor::init() {

    spdlog::debug("Created PlayheadGlobalEventsActor {}", name());
    print_on_exit(this, "PlayheadGlobalEventsActor");

    system().registry().put(global_playhead_events_actor, this);

    event_group_             = spawn<broadcast::BroadcastActor>(this);
    fine_grain_events_group_ = spawn<broadcast::BroadcastActor>(this);

    link_to(event_group_);

    set_down_handler([=](down_msg &msg) {
        // find in playhead list..
        auto p = viewport_playheads_.begin();
        while (p != viewport_playheads_.end()) {
            if (msg.source == p->second) {
                demonitor(p->second);
                p = viewport_playheads_.erase(p);
            }
        }
    });

    behavior_.assign(

        [=](broadcast::broadcast_down_atom, const caf::actor_addr &) {},
        [=](const group_down_msg & /*msg*/) {},
        [=](utility::get_event_group_atom) -> caf::actor { return event_group_; },
        [=](broadcast::join_broadcast_atom, caf::actor joiner) {
            delegate(event_group_, broadcast::join_broadcast_atom_v, joiner);
        },
        [=](broadcast::leave_broadcast_atom, caf::actor joiner) {
            delegate(event_group_, broadcast::leave_broadcast_atom_v, joiner);
        },
        [=](ui::viewport::viewport_playhead_atom) -> caf::actor {
            return global_active_playhead_;
        },
        [=](ui::viewport::viewport_playhead_atom, caf::actor playhead) {
            // something can send us this message in order to set the 'global'
            // playhead - i.e. the playhead that is being viewed by non-quickview
            // viewports. SessionModel::setPlayheadTo does this for example.
            for (auto &p : viewports_) {
                auto viewport_actor = caf::actor_cast<caf::actor>(p.second);
                if (viewport_actor) {
                    anon_send(viewport_actor, ui::viewport::viewport_playhead_atom_v, playhead);
                }
            }
            global_active_playhead_ = playhead;
        },
        [=](ui::viewport::viewport_playhead_atom,
            const std::string viewport_name,
            caf::actor playhead) {
            send(
                event_group_,
                utility::event_atom_v,
                ui::viewport::viewport_playhead_atom_v,
                viewport_name,
                playhead);

            if (viewport_playheads_[viewport_name] &&
                viewport_playheads_[viewport_name] != playhead) {
                bool playhead_still_active = false;
                for (auto &p : viewport_playheads_) {
                    if (p.second == playhead) {
                        playhead_still_active = true;
                    }
                }
                if (!playhead_still_active) {
                    demonitor(viewport_playheads_[viewport_name]);
                }
                viewport_playheads_[viewport_name] = playhead;
                if (playhead)
                    monitor(playhead);
            } else {
                bool playhead_already_monitored = false;
                for (auto &p : viewport_playheads_) {
                    if (p.second == playhead) {
                        playhead_already_monitored = true;
                    }
                }
                if (!playhead_already_monitored && playhead)
                    monitor(playhead);
                viewport_playheads_[viewport_name] = playhead;
            }
        },

        [=](show_atom, const media_reader::ImageBufPtr &buf) {
            // a playhead is telling us a new frame is being shown.

            // Forward the info to our 'fine grain' message group with details
            // of which viewport the frame is being shown on
            auto playhead = caf::actor_cast<caf::actor>(current_sender());
            for (auto &p : viewport_playheads_) {
                if (p.second == playhead) {
                }
            }
        },
        [=](show_atom, caf::actor media, caf::actor media_source) {
            // a playhead is telling us the on-screen media has changed
            auto playhead = caf::actor_cast<caf::actor>(current_sender());
            for (auto &p : viewport_playheads_) {
                if (p.second == playhead) {
                    // forward the event, including the name of the viewport(s)
                    // that are attached to the playhead
                    send(
                        event_group_,
                        utility::event_atom_v,
                        show_atom_v,
                        media,
                        media_source,
                        p.first);
                }
            }
        },
        [=](ui::viewport::viewport_playhead_atom,
            const std::string viewport_name) -> caf::actor {
            if (viewports_.find(viewport_name) != viewports_.end()) {
                return viewport_playheads_[viewport_name];
            }
            return caf::actor();
        },
        [=](ui::viewport::viewport_atom, const std::string viewport_name, caf::actor viewport) {
            // viewports register themselves by sending us this message
            viewports_[viewport_name] = caf::actor_cast<caf::actor_addr>(viewport);
            send(
                event_group_,
                utility::event_atom_v,
                ui::viewport::viewport_atom_v,
                viewport_name,
                viewport);
        },
        [=](ui::viewport::viewport_atom,
            const std::string viewport_name) -> result<caf::actor> {
            // Here we can request a named viewport
            caf::actor r;
            auto p = viewports_.find(viewport_name);
            if (p != viewports_.end()) {
                r = caf::actor_cast<caf::actor>(p->second);
            }
            if (!r)
                return make_error(
                    xstudio_error::error, fmt::format("No viewport named {}", viewport_name));
            return r;
        });
}