// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>
#include "xstudio/timeline/item.hpp"
#include "xstudio/utility/uuid.hpp"
#include <string>

namespace xstudio {
namespace timeline {

    /*This class implements features that are common requirements
    of various 'actor' based components within the xSTUDIO code
    base.
    
    Convenience functions are provided to:
    
        - Retrieve actors from the CAF registry
        - Lookup, watch and automatically track preference values
    */

    class ItemActor : public caf::event_based_actor {

      public:

        ItemActor(caf::actor_config &cfg, Item &i);
        ~ItemActor() override = default;

        virtual caf::message_handler message_handler() = 0;

      protected:

        caf::actor & event_group() { return event_group_; }

      private:

        caf::behavior make_behavior() override;

      private:

        caf::behavior base_behavior_;
        caf::actor event_group_;
        Item &item_;

    };

} // namespace broadcast
} // namespace xstudio
