// SPDX-License-Identifier: Apache-2.0

#include "xstudio/atoms.hpp"
#include "xstudio/timeline/item_actor.hpp"
#include "xstudio/timeline/gap_actor.hpp"
#include "xstudio/timeline/clip_actor.hpp"
#include "xstudio/timeline/track_actor.hpp"
#include "xstudio/timeline/stack_actor.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/container.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/broadcast/broadcast_actor.hpp"

using namespace xstudio;
using namespace xstudio::timeline;
using namespace xstudio::utility;

ItemActor0::ItemActor0(caf::actor_config &cfg) : utility::XStudioActor(cfg) {

    extend_base_behaviour(
        [=](name_atom, const std::string &name) { // make_set_name_handler
            base().set_name(name);
            send_event(event_atom_v, name_atom_v, name);
        },
        [=](name_atom) -> std::string { return base().name(); }, // make_get_name_handler
        [=](uuid_atom) -> Uuid { return base().uuid(); },        // make_get_uuid_handler
        [=](type_atom) -> std::string { 
            auto p = timeline_type_names.find(base().item_type());
            if (p != timeline_type_names.end()) return p->second;
            return "Unknown"; 
        }, // make_get_type_handler
        [=](session::describe_atom) -> utility::JsonStore {
            if (description_.is_null()) {
                description_ = base().serialise();                
            }
            return description_;
        },
        [=](detail_atom) -> ContainerDetail {
            std::string type("Unknown"); 
            auto p = timeline_type_names.find(base().item_type());
            if (p != timeline_type_names.end()) type = p->second;
            return utility::ContainerDetail(base().name(), type, base().uuid(), caf::actor_cast<caf::actor>(this), event_group());
        },
        [=](all_children_atom) -> result <utility::UuidActorVector> {
            // Here recursively fetch all children ItemActors into a flat
            // list.
            auto auto_responder = AutoResponder<utility::UuidActorVector>(child_item_actors_.size(), this);            
            for (const auto &i : child_item_actors_) {
                auto_responder.result().push_back(utility::UuidActor(i.first, i.second));
                mail(all_children_atom_v).request(i.second, infinite).then(
                    [=](const utility::UuidActorVector &grandkids) mutable {
                        auto_responder.result().insert(auto_responder.result().begin(), grandkids.begin(), grandkids.end());
                        auto_responder.decrement();
                    },
                    [=](caf::error & e) mutable {
                        auto_responder.decrement(e);
                    });

            }
            return auto_responder.response_promise();
        });
}

void ItemActor0::on_exit() {
    for (const auto &i : child_item_actors_)
        send_exit(i.second, caf::exit_reason::user_shutdown);
}

void ItemActor0::add_child_item(const UuidActor &ua) {
    // join_event_group(this, ua.second);
    scoped_actor sys{system()};

    try {
        auto grp    = utility::request_receive<caf::actor>(*sys, ua.actor(), get_event_group_atom_v);
        auto joined = request_receive<bool>(*sys, grp, broadcast::join_broadcast_atom_v, this);
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }

    auto act_addr = caf::actor_cast<caf::actor_addr>(ua.actor());

    if (auto sit = monitor_.find(act_addr); sit == std::end(monitor_)) {
        monitor_[act_addr] =
            monitor(ua.actor(), [this, addr = ua.actor().address()](const error &) {
                if (auto mit = monitor_.find(caf::actor_cast<caf::actor_addr>(addr));
                    mit != std::end(monitor_))
                    monitor_.erase(mit);

                for (auto it = std::begin(child_item_actors_); it != std::end(child_item_actors_); ++it) {
                    if (addr == it->second) {
                        child_item_actors_.erase(it);

                        // remove from base.
                        auto it = find_actor_addr(base().children(), addr);

                        if (it != base().end()) {
                            auto jsn  = base().erase(it);
                            auto more = base().refresh();
                            if (not more.is_null())
                                jsn.insert(jsn.begin(), more.begin(), more.end());

                            mail(event_atom_v, item_atom_v, jsn, false)
                                .send(event_group());
                        }

                        break;
                    }
                }
            });
    }

    child_item_actors_[ua.uuid()] = ua.actor();
}

void ItemActor0::make_child_item_actors() {

    for (auto &i : base().children()) {

        if (child_item_actors_.find(i.uuid()) != child_item_actors_.end()) continue;

        switch (i.item_type()) {
            case IT_CLIP: {
                auto actor = spawn<ClipActor>(i);
                add_child_item(UuidActor(i.uuid(), actor));
            } break;
            case IT_GAP: {
                auto actor = spawn<GapActor>(i);
                add_child_item(UuidActor(i.uuid(), actor));
            } break;
            case IT_STACK: {
                auto actor = spawn<StackActor>(i);
                add_child_item(UuidActor(i.uuid(), actor));
            } break;
            case IT_AUDIO_TRACK:
            case IT_VIDEO_TRACK: {
                auto actor = spawn<TrackActor>(i);
                add_child_item(UuidActor(i.uuid(), actor));
            } break;
            default:
                break;
        }
    }

}

