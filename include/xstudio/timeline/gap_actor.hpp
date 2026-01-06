// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>

#include "xstudio/timeline/gap.hpp"
#include "xstudio/timeline/item_actor.hpp"

namespace xstudio {
namespace timeline {
    class GapActor : public ItemActor2<Gap> {
      public:
        GapActor(caf::actor_config &cfg, const utility::JsonStore &jsn);
        GapActor(caf::actor_config &cfg, const utility::JsonStore &jsn, Item &item);
        GapActor(
            caf::actor_config &cfg,
            const std::string &name                    = "Gap",
            const utility::FrameRateDuration &duration = utility::FrameRateDuration(),
            const utility::Uuid &uuid                  = utility::Uuid::generate());

        GapActor(caf::actor_config &cfg, const Item &item);
        GapActor(caf::actor_config &cfg, const Item &item, Item &nitem);

        const char *name() const override { return NAME.c_str(); }

      private:
        inline static const std::string NAME = "GapActor";
        void init();

        caf::message_handler message_handler() override;

    };
} // namespace timeline
} // namespace xstudio
