// SPDX-License-Identifier: Apache-2.0

#include "xstudio/atoms.hpp"
#include "xstudio/timeline/item_actor.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/container.hpp"
#include "xstudio/broadcast/broadcast_actor.hpp"

using namespace xstudio;
using namespace xstudio::timeline;
using namespace xstudio::utility;

ItemActor::ItemActor(caf::actor_config &cfg, Item &item) : caf::event_based_actor(cfg), item_(item) {

    event_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(event_group_);

    base_behavior_ = {
        [=](name_atom, const std::string &name) { // make_set_name_handler
            item_.set_name(name);
            mail(event_atom_v, name_atom_v, name).send(event_group_);
        },
        [=](name_atom) -> std::string { return item_.name(); }, // make_get_name_handler
        [=](uuid_atom) -> Uuid { return item_.uuid(); },        // make_get_uuid_handler
        [=](type_atom) -> std::string { 
            auto p = timeline_type_names.find(item_.item_type());
            if (p != timeline_type_names.end()) return p->second;
            return "Unknown"; 
        }, // make_get_type_handler
        [=](get_event_group_atom) -> caf::actor {
            return event_group_;
        },
        [=](detail_atom) -> ContainerDetail {
            std::string type("Unknown"); 
            auto p = timeline_type_names.find(item_.item_type());
            if (p != timeline_type_names.end()) type = p->second;
            return utility::ContainerDetail(item_.name(), type, item_.uuid(), caf::actor_cast<caf::actor>(this), event_group_);
        }
    };
}

caf::behavior ItemActor::make_behavior() { 
    
    return message_handler().or_else(base_behavior_); 

}

