// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>
#include <chrono>

#include "xstudio/utility/container.hpp"
#include "xstudio/utility/xs_actor.hpp"

namespace xstudio {

namespace playlist {

    class PlaylistDividerActor : public xstudio::utility::ObjectActor, utility::Container {
        public:

        PlaylistDividerActor(
            caf::actor_config &cfg,
            const utility::JsonStore &jsn,
            const caf::actor &parent = caf::actor());

        PlaylistDividerActor(
            caf::actor_config &cfg,
            const std::string &name,
            const utility::Uuid &uuid = utility::Uuid(),
            const caf::actor &parent = caf::actor());

        ~PlaylistDividerActor() override = default;

        [[nodiscard]] const char *name() const override { return NAME.c_str(); }
        
        protected:

        caf::message_handler extra_message_handlers() override {
            return Container::container_message_handler(this);
        }

        private:
        inline static const std::string NAME = "PlaylistDividerActor";
    };

} // namespace playlist

} // namespace xstudio