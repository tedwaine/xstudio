// SPDX-License-Identifier: Apache-2.0

#include <caf/actor_registry.hpp>

#include "xstudio/atoms.hpp"
#include "xstudio/utility/base_actor.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/broadcast/broadcast_actor.hpp"

using namespace xstudio;
using namespace xstudio::utility;

XStudioActor::XStudioActor(caf::actor_config &cfg) : caf::event_based_actor(cfg) {

    event_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(event_group_);

    base_message_handler_ = {
        [=](get_event_group_atom) -> caf::actor {
            return event_group_;
        }
    };
}

caf::behavior XStudioActor::make_behavior() { 
    
    return message_handler().or_else(base_message_handler_); 

}

caf::actor XStudioActor::registered_actor(const std::string & registry_name) const {
    return system().registry().get<caf::actor>(registry_name);
}

void XStudioActor::send_changed(const time_point last_changed) {
    if (last_changed > last_changed_) {
        last_changed_ = std::move(last_changed);
        send_event(event_atom_v, last_changed_atom_v, last_changed_);
    }
}
