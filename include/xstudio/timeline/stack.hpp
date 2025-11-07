// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/actor.hpp>
#include <string>

#include "xstudio/timeline/item.hpp"
#include "xstudio/utility/container.hpp"
#include "xstudio/utility/frame_rate.hpp"
#include "xstudio/utility/json_store.hpp"
#include "xstudio/utility/uuid.hpp"

namespace xstudio {
namespace timeline {
    class Stack : public Item {
      public:
        Stack(
            const std::string &name        = "Stack",
            const utility::FrameRate &rate = utility::FrameRate(),
            const utility::Uuid &uuid      = utility::Uuid::generate(),
            const caf::actor &actor        = caf::actor());

        Stack(const utility::JsonStore &jsn);
        Stack(const Item &item, const caf::actor &actor);

        [[nodiscard]] Stack duplicate() const;

    };
} // namespace timeline
} // namespace xstudio