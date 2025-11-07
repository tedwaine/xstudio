// SPDX-License-Identifier: Apache-2.0

#include "xstudio/atoms.hpp"
#include "xstudio/utility/base_actor.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/broadcast/broadcast_actor.hpp"

using namespace xstudio;
using namespace xstudio::utility;

XStudioActor::XStudioActor(caf::actor_config &cfg) : caf::event_based_actor(cfg) {

    event_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(event_group_);

    base_behavior_ = {
        [=](name_atom, const std::string &name) { // make_set_name_handler
            name_ = name;
            mail(event_atom_v, name_atom_v, name).send(event_group_);
        },
        [=](name_atom) -> std::string { return name_; }, // make_get_name_handler
        [=](uuid_atom) -> Uuid { return uuid_; },        // make_get_uuid_handler
        [=](type_atom) -> std::string { return type_; }, // make_get_type_handler
        [=](get_event_group_atom) -> caf::actor {
            return event_group_;
        }
    };
}

caf::behavior XStudioActor::make_behavior() { 
    
    return message_handler().or_else(base_behavior_); 

}

