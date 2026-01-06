// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/actor.hpp>
#include <string>

#include "xstudio/timeline/item.hpp"
#include "xstudio/utility/container.hpp"
#include "xstudio/utility/frame_rate_and_duration.hpp"
#include "xstudio/utility/json_store.hpp"
#include "xstudio/utility/uuid.hpp"

namespace xstudio {
namespace timeline {
    class Gap : public Item {
      public:
        Gap(const std::string &name                    = "Gap",
            const utility::FrameRateDuration &duration = utility::FrameRateDuration(),
            const utility::Uuid &uuid                  = utility::Uuid::generate());
        Gap(const utility::JsonStore &jsn);
        Gap(const Item &item);

        [[nodiscard]] Gap duplicate() const;

    };
} // namespace timeline
} // namespace xstudio