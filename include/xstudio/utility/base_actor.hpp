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

      private:

        caf::behavior make_behavior() override;

      private:

        caf::behavior base_behavior_;
        caf::actor event_group_;
        std::string name_;
        std::string type_;
        utility::Uuid uuid_;

    };

} // namespace broadcast
} // namespace xstudio
