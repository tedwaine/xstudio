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

    class ItemActor0 : public utility::XStudioActor {

      public:

        ItemActor0(caf::actor_config &cfg);
        ~ItemActor0() override = default;

        void on_exit() override;

      protected:

        void add_child_item(const utility::UuidActor &ua);

        std::map<utility::Uuid, caf::actor> child_item_actors_;
        std::map<caf::actor_addr, caf::disposable> monitor_;

        void make_child_item_actors();

        virtual Item & base() = 0;

      private:

        utility::JsonStore description_;

    };

    class ItemActor : public ItemActor0 {

      public:

        ItemActor(caf::actor_config &cfg, Item &i) : ItemActor0(cfg), item_(i) {}

      protected:

        Item & base() override { return item_; }

      private:

        Item & item_;

    };

    template<class ITEM_TYPE>
    class ItemActor2 : public ItemActor0, public ITEM_TYPE {

      public:

        template <class... Ts>
        ItemActor2(caf::actor_config &cfg, Ts&&... xs) : 
          ItemActor0(cfg), 
          ITEM_TYPE(std::forward<Ts>(xs)...) {
            ITEM_TYPE::set_actor_addr(as_actor());
            ItemActor0::make_child_item_actors();
          }

      protected:

        Item & base() override { return static_cast<Item &>(*this); }

    };

} // namespace broadcast
} // namespace xstudio
