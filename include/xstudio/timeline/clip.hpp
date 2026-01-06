// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/actor.hpp>
#include <string>

#include "xstudio/timeline/item.hpp"
#include "xstudio/utility/json_store.hpp"
#include "xstudio/utility/uuid.hpp"

namespace xstudio {
namespace timeline {

    class Clip : public Item {

      public:

        Clip(
            const std::string name         = "Clip",
            const utility::Uuid &uuid      = utility::Uuid::generate(),
            const utility::Uuid media_uuid = utility::Uuid());
        Clip(const utility::JsonStore &jsn);
        Clip(const Item &item, const caf::actor &actor);

        [[nodiscard]] utility::JsonStore serialise() const;
        [[nodiscard]] Clip duplicate() const;

        [[nodiscard]] const utility::Uuid &media_uuid() const { return media_uuid_; }
        utility::JsonStore set_media_uuid(const utility::Uuid &media_uuid) {
            auto jsn          = Item::prop();
            jsn["media_uuid"] = media_uuid;
            media_uuid_       = media_uuid;
            return Item::set_prop(jsn);
        }

        void override_media_rate(const utility::FrameRate &media_rate) {
            overridden_media_rate_ = media_rate;
        }

        [[nodiscard]] const utility::FrameRate &media_rate() const {
            return overridden_media_rate_;
        }

      private:

        utility::Uuid media_uuid_;
        utility::FrameRate overridden_media_rate_;
    };
} // namespace timeline
} // namespace xstudio