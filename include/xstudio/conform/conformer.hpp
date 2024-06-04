// SPDX-License-Identifier: Apache-2.0

#pragma once

// #include <array>
// #include <caf/uri.hpp>
// #include <cstddef>
// #include <map>
// #include <nlohmann/json.hpp>
// #include <optional>
// #include <string>

#include "xstudio/global_store/global_store.hpp"
#include "xstudio/plugin_manager/plugin_factory.hpp"
#include "xstudio/timeline/item.hpp"
#include "xstudio/utility/media_reference.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/helpers.hpp"

namespace xstudio {
namespace conform {

    // item json, we might need to expand this with more detail, may need to support clips.
    // might need a custom handler on items to generate more usful hints.

    // item, metadata, insert before this in media list.
    const auto ConformOperationsJSON = R"({
        "create_media": false,
        "remove_media": false,
        "insert_media": false,
        "new_track_name": "",
        "remove_failed_clip": false,
        "replace_clip": false
    })"_json;


    // item, media, before
    typedef std::tuple<utility::UuidActor, utility::UuidActor, utility::Uuid>
        ConformRequestItem;

    struct ConformRequest {
        ConformRequest(
            const utility::UuidActor playlist,
            const utility::UuidActor container,
            const std::string item_type,
            const std::vector<ConformRequestItem> items)
            : playlist_(std::move(playlist)),
              container_(std::move(container)),
              item_type_(std::move(item_type)),
              items_(std::move(items)),
              operations_(ConformOperationsJSON) {}
        ConformRequest(
            const utility::UuidActor playlist,
            const utility::UuidActor container,
            const timeline::Item track,
            const std::vector<ConformRequestItem> items)
            : playlist_(std::move(playlist)),
              container_(std::move(container)),
              track_(std::move(track)),
              item_type_("Clip"),
              items_(std::move(items)),
              operations_(ConformOperationsJSON) {}

        ConformRequest() : operations_(ConformOperationsJSON) {}
        ~ConformRequest() = default;

        utility::UuidActor playlist_;
        utility::UuidActor container_;
        std::string item_type_;
        std::vector< // request item
            ConformRequestItem>
            items_;

        utility::JsonStore operations_;
        utility::JsonStore detail_;
        std::map<utility::Uuid, utility::JsonStore> metadata_;
        timeline::Item track_;


        template <class Inspector> friend bool inspect(Inspector &f, ConformRequest &x) {
            return f.object(x).fields(
                f.field("pl", x.playlist_),
                f.field("ct", x.container_),
                f.field("op", x.operations_),
                f.field("dt", x.detail_),
                f.field("cm", x.metadata_),
                f.field("it", x.item_type_),
                f.field("t", x.track_),
                f.field("items", x.items_));
        }

        void dump() const {
            spdlog::warn("playlist {}", to_string(playlist_));
            spdlog::warn("container {}", to_string(container_));
            spdlog::warn("item type {}", item_type_);
            for (const auto &i : items_) {
                spdlog::warn(
                    "item {} media {} before {}",
                    to_string(std::get<0>(i)),
                    to_string(std::get<1>(i)),
                    to_string(std::get<2>(i)));
            }
            for (const auto &i : metadata_) {
                spdlog::warn("metadata {} {}", to_string(i.first), i.second.dump(2));
            }
        }
    };

    typedef std::tuple<utility::UuidActor // reference to matches media actor or clip actors.
                       >
        ConformReplyItem;

    struct ConformReply {
        ConformReply() : operations_(ConformOperationsJSON) {}
        ConformReply(ConformRequest request)
            : operations_(ConformOperationsJSON), request_(std::move(request)) {}
        ~ConformReply() = default;

        ConformRequest request_;
        utility::JsonStore operations_;
        std::vector<std::optional<std::vector<ConformReplyItem>>> items_;

        template <class Inspector> friend bool inspect(Inspector &f, ConformReply &x) {
            return f.object(x).fields(
                f.field("req", x.request_),
                f.field("op", x.operations_),
                f.field("items", x.items_));
        }
    };

    class Conformer {
      public:
        Conformer(const utility::JsonStore &prefs = utility::JsonStore());
        virtual ~Conformer() = default;
        virtual void update_preferences(const utility::JsonStore &prefs);

        virtual ConformReply
        conform_request(const std::string &conform_task, const ConformRequest &request);

        virtual ConformReply conform_request(const ConformRequest &request);
        virtual bool conform_prepare_timeline(const utility::UuidActor &timeline);

        virtual std::vector<std::string> conform_tasks();
    };

    template <typename T>
    class ConformPlugin : public plugin_manager::PluginFactoryTemplate<T> {
      public:
        ConformPlugin(
            utility::Uuid uuid,
            std::string name        = "",
            std::string author      = "",
            std::string description = "",
            semver::version version = semver::version("0.0.0"))
            : plugin_manager::PluginFactoryTemplate<T>(
                  uuid,
                  name,
                  plugin_manager::PluginType(plugin_manager::PluginFlags::PF_CONFORM),
                  false,
                  author,
                  description,
                  version) {}
        ~ConformPlugin() override = default;
    };

    template <typename T> class ConformPluginActor : public caf::event_based_actor {

      public:
        ConformPluginActor(
            caf::actor_config &cfg, const utility::JsonStore &jsn = utility::JsonStore())
            : caf::event_based_actor(cfg), conform_(jsn) {

            spdlog::debug("Created ConformPluginActor");
            utility::print_on_exit(this, "ConformPluginActor");

            {
                auto prefs = global_store::GlobalStoreHelper(system());
                utility::JsonStore js;
                utility::join_broadcast(this, prefs.get_group(js));
                conform_.update_preferences(js);
            }

            behavior_.assign(
                [=](xstudio::broadcast::broadcast_down_atom, const caf::actor_addr &) {},

                [=](conform_atom,
                    const std::string &conform_task,
                    const ConformRequest &request) -> ConformReply {
                    return conform_.conform_request(conform_task, request);
                },

                [=](conform_atom, const ConformRequest &request) -> ConformReply {
                    return conform_.conform_request(request);
                },

                [=](conform_atom, const utility::UuidActor &timeline) -> bool {
                    return conform_.conform_prepare_timeline(timeline);
                },

                [=](conform_tasks_atom) -> std::vector<std::string> {
                    return conform_.conform_tasks();
                },

                [=](json_store::update_atom,
                    const utility::JsonStore & /*change*/,
                    const std::string & /*path*/,
                    const utility::JsonStore &full) {
                    delegate(actor_cast<caf::actor>(this), json_store::update_atom_v, full);
                },

                [=](json_store::update_atom, const utility::JsonStore &js) {
                    try {
                        conform_.update_preferences(js);
                    } catch (const std::exception &err) {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    }
                });
        }

        caf::behavior make_behavior() override { return behavior_; }

      private:
        caf::behavior behavior_;
        T conform_;
    };

} // namespace conform
} // namespace xstudio
