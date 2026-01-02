// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>
#include "xstudio/timeline/item.hpp"
#include "xstudio/utility/uuid.hpp"
#include "xstudio/utility/base_actor.hpp"
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

    class ItemActor : public utility::XStudioActor {

      public:

        ItemActor(caf::actor_config &cfg, Item &i);
        ~ItemActor() override = default;

      private:

        Item &item_;

    };

} // namespace broadcast
} // namespace xstudio
