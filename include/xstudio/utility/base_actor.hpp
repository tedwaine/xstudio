// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>
#include "xstudio/utility/uuid.hpp"
#include <string>

namespace xstudio {
namespace utility {

    /*This class implements features that are common requirements
    of various 'actor' based components within the xSTUDIO code
    base.
    
    Convenience functions are provided to:
    
        - Retrieve actors from the CAF registry
        - Lookup, watch and automatically track preference values
    */

    class XStudioActor : public caf::event_based_actor {

      public:

        XStudioActor(caf::actor_config &cfg);
        ~XStudioActor() override = default;

        virtual caf::message_handler message_handler() = 0;

      protected:

        caf::actor & event_group() { return event_group_; }

        // The method allows us to build the overall message handler
        // set for an actor through C++ inheritance. If actor class
        // B inherits from A, and A inherits from XStudioActor then
        // in class A constructor we run extend_base_behaviour with
        // a set of message handlers and in class B we also call
        // extend_base_behaviour with another set of message handlers
        // then class B will have a merge of class A and class B
        // handlers.
        template <class... Ts>
        void extend_base_behaviour(Ts&&... xs) {
          caf::message_handler tmp{std::forward<Ts>(xs)...};          
          base_message_handler_.assign(tmp.or_else(base_message_handler_));
        }

        // Helper function to send an event message via the event_group.
        // Simply pass in the message elements as arguments as though
        // using caf::mail(msg).send(event_group).
        //
        // For example:
        //
        // send_event(utility::event_atom_v, utility::name_atom_v, new_name);
        template <class... Ts>
        void send_event(Ts&&... xs) {
          mail(std::forward<Ts>(xs)...).send(event_group());
        }

        // Helper function to retrieve one of the 'registered' core actors 
        // from the xstudio actor system.
        caf::actor registered_actor(const std::string & registry_name) const;

        void send_changed(const time_point last_changed = utility::clock::now());

      private:

        caf::behavior make_behavior() override;

        caf::behavior base_message_handler_;
        caf::actor event_group_;
        time_point last_changed_ = {utility::clock::now()};

    };

} // namespace broadcast
} // namespace xstudio
