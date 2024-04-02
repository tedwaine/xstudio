// SPDX-License-Identifier: Apache-2.0
#include <caf/sec.hpp>
#include <caf/policy/select_all.hpp>
#include <caf/policy/select_any.hpp>

#include "xstudio/atoms.hpp"
#include "xstudio/global_store/global_store.hpp"
#include "xstudio/conform/conformer.hpp"
#include "xstudio/conform/conform_manager_actor.hpp"
#include "xstudio/plugin_manager/plugin_factory.hpp"
#include "xstudio/plugin_manager/plugin_manager.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/json_store.hpp"
#include "xstudio/utility/logging.hpp"

using namespace xstudio;
using namespace std::chrono_literals;
using namespace xstudio::utility;
using namespace xstudio::json_store;
using namespace xstudio::global_store;
using namespace xstudio::conform;
using namespace caf;

ConformWorkerActor::ConformWorkerActor(caf::actor_config &cfg) : caf::event_based_actor(cfg) {

    std::vector<caf::actor> conformers;

    // get hooks
    {
        auto pm = system().registry().template get<caf::actor>(plugin_manager_registry);
        scoped_actor sys{system()};
        auto details = request_receive<std::vector<plugin_manager::PluginDetail>>(
            *sys,
            pm,
            utility::detail_atom_v,
            plugin_manager::PluginType(plugin_manager::PluginFlags::PF_CONFORM));

        for (const auto &i : details) {
            if (i.enabled_) {
                auto actor = request_receive<caf::actor>(
                    *sys, pm, plugin_manager::spawn_plugin_atom_v, i.uuid_);
                link_to(actor);
                conformers.push_back(actor);
            }
        }
    }

    // distribute to all conformers.

    behavior_.assign(
        [=](xstudio::broadcast::broadcast_down_atom, const caf::actor_addr &) {},

        [=](conform_tasks_atom) -> result<std::vector<std::string>> {
            if (not conformers.empty()) {
                auto rp = make_response_promise<std::vector<std::string>>();
                fan_out_request<policy::select_all>(conformers, infinite, conform_tasks_atom_v)
                    .then(
                        [=](const std::vector<std::vector<std::string>> all_results) mutable {
                            // compile results..
                            auto dups    = std::set<std::string>();
                            auto results = std::vector<std::string>();

                            for (const auto &i : all_results) {
                                for (const auto &j : i) {
                                    if (dups.count(j))
                                        continue;

                                    dups.insert(j);
                                    results.push_back(j);
                                }
                            }

                            rp.deliver(results);
                        },
                        [=](const error &err) mutable { rp.deliver(err); });
                return rp;
            }

            return std::vector<std::string>();
        },

        [=](conform_atom,
            const std::string &conform_task,
            const utility::JsonStore &conform_detail,
            const UuidActor &playlist,
            const UuidActorVector &media,
            const UuidVector &insert_before) -> result<ConformReply> {
            // make worker gather all the information
            auto rp = make_response_promise<ConformReply>();

            request(playlist.actor(), infinite, json_store::get_json_atom_v, "")
                .then(
                    [=](const JsonStore &playlist_json) mutable {
                        // get all media json..
                        // we'd also like to know the current source name...
                        // ack!

                        fan_out_request<policy::select_all>(
                            vector_to_caf_actor_vector(media),
                            infinite,
                            json_store::get_json_atom_v,
                            utility::Uuid(),
                            "",
                            true)
                            .then(
                                [=](const std::vector<std::pair<UuidActor, JsonStore>>
                                        media_json_reply) mutable {
                                    // also get source names..
                                    fan_out_request<policy::select_all>(
                                        vector_to_caf_actor_vector(media),
                                        infinite,
                                        media::current_media_source_atom_v,
                                        true)
                                        .then(
                                            [=](const std::vector<std::pair<
                                                    UuidActor,
                                                    std::pair<std::string, std::string>>>
                                                    media_source_reply) mutable {
                                                // reorder into Conform request.
                                                auto media_json =
                                                    std::vector<ConformRequestItem>();
                                                std::map<Uuid, JsonStore> jsn_map;
                                                std::map<
                                                    Uuid,
                                                    std::pair<std::string, std::string>>
                                                    source_map;
                                                for (const auto &i : media_source_reply)
                                                    source_map[i.first.uuid()] = i.second;

                                                for (const auto &i : media_json_reply) {
                                                    jsn_map[i.first.uuid()] = i.second;
                                                    jsn_map[i.first.uuid()]["metadata"]
                                                           ["image_source"] =
                                                               source_map.at(i.first.uuid())
                                                                   .first;
                                                    jsn_map[i.first.uuid()]["metadata"]
                                                           ["audio_source"] =
                                                               source_map.at(i.first.uuid())
                                                                   .second;
                                                }

                                                size_t count = 0;
                                                for (const auto &i : media) {
                                                    auto before = Uuid();
                                                    if (insert_before.size() > count) {
                                                        before = insert_before.at(count);
                                                    }
                                                    count++;

                                                    media_json.emplace_back(std::make_tuple(
                                                        i, jsn_map.at(i.uuid()), before));
                                                }

                                                rp.delegate(
                                                    caf::actor_cast<caf::actor>(this),
                                                    conform_atom_v,
                                                    conform_task,
                                                    conform_detail,
                                                    ConformRequest(
                                                        playlist, playlist_json, media_json));
                                            },
                                            [=](const error &err) mutable {
                                                spdlog::warn(
                                                    "ONE {} {}",
                                                    __PRETTY_FUNCTION__,
                                                    to_string(err));
                                                rp.deliver(err);
                                            });
                                },
                                [=](const error &err) mutable {
                                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                                    rp.deliver(err);
                                });
                    },
                    [=](const error &err) mutable {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                        rp.deliver(err);
                    });

            return rp;
        },

        // [=](current_media_source_atom, const bool current_names) -> caf::result
        // <std::tuple<UuidActor, std::string, std::string>> {

        [=](conform_atom,
            const std::string &conform_task,
            const utility::JsonStore &conform_detail,
            const ConformRequest &request) -> result<ConformReply> {
            if (not conformers.empty()) {
                auto rp = make_response_promise<ConformReply>();
                fan_out_request<policy::select_all>(
                    conformers, infinite, conform_atom_v, conform_task, conform_detail, request)
                    .then(
                        [=](const std::vector<ConformReply> all_results) mutable {
                            // compile results..
                            auto result = ConformReply();
                            result.items_.resize(request.items_.size());

                            for (const auto &i : all_results) {
                                if (not i.items_.empty()) {
                                    // insert values into result.
                                    auto count = 0;
                                    for (const auto &j : i.items_) {
                                        // replace, don't sum results, so we only expect one
                                        // result set in total from a plugin.
                                        if (j and not result.items_[count])
                                            result.items_[count] = j;
                                        count++;
                                    }
                                }
                            }

                            rp.deliver(result);
                        },
                        [=](const error &err) mutable { rp.deliver(err); });
                return rp;
            }

            return ConformReply();
        });
}

ConformManagerActor::ConformManagerActor(caf::actor_config &cfg, const utility::Uuid uuid)
    : caf::event_based_actor(cfg), uuid_(std::move(uuid)), module::Module("ConformManager") {
    spdlog::debug("Created ConformManagerActor.");
    print_on_exit(this, "ConformManagerActor");

    try {
        auto prefs = GlobalStoreHelper(system());
        JsonStore j;
        join_broadcast(this, prefs.get_group(j));
        worker_count_ = preference_value<size_t>(j, "/core/conform/max_worker_count");
    } catch (...) {
    }

    spdlog::debug("ConformManagerActor worker_count {}", worker_count_);

    event_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(event_group_);

    pool_ = caf::actor_pool::make(
        system().dummy_execution_unit(),
        worker_count_,
        [&] { return system().spawn<ConformWorkerActor>(); },
        caf::actor_pool::round_robin());
    link_to(pool_);

    system().registry().put(conform_registry, this);

    make_behavior();
    connect_to_ui();

    data_.set_origin(true);
    data_.bind_send_event_func([&](auto &&PH1, auto &&PH2) {
        auto event     = JsonStore(std::forward<decltype(PH1)>(PH1));
        auto undo_redo = std::forward<decltype(PH2)>(PH2);

        send(event_group_, utility::event_atom_v, json_store::sync_atom_v, data_uuid_, event);
    });

    // my_menu_      = insert_menu_item("media_list_menu_", "Conform", "", 0.0f);
    // compare_menu_ = insert_menu_item("media_list_menu_", "Compare", "Conform", 0.0f);
    // replace_menu_ = insert_menu_item("media_list_menu_", "Replace", "Conform", 0.0f);

    // next_menu_item_ = insert_menu_item("media_list_menu_", "Next Version", "Conform", 0.0f);
    // previous_menu_item_ =
    //     insert_menu_item("media_list_menu_", "Previous Version", "Conform", 0.0f);
    // latest_menu_item_ = insert_menu_item("media_list_menu_", "Latest Version", "Conform",
    // 0.0f);

    // trigger request for tasks..
    delayed_anon_send(
        caf::actor_cast<caf::actor>(this), std::chrono::seconds(5), conform_tasks_atom_v);
}

caf::message_handler ConformManagerActor::message_handler_extensions() {
    return caf::message_handler(
        make_get_event_group_handler(event_group_),
        [=](xstudio::broadcast::broadcast_down_atom, const caf::actor_addr &) {},

        [=](utility::event_atom,
            json_store::sync_atom,
            const Uuid &uuid,
            const JsonStore &event) {
            if (uuid == data_uuid_)
                data_.process_event(event, true, false, false);
        },

        [=](json_store::sync_atom) -> UuidVector { return UuidVector({data_uuid_}); },

        [=](json_store::sync_atom, const Uuid &uuid) -> JsonStore {
            if (uuid == data_uuid_)
                return data_.as_json();

            return JsonStore();
        },


        [=](conform_atom,
            const std::string &conform_task,
            const utility::JsonStore &conform_detail,
            const ConformRequest &request) {
            delegate(pool_, conform_atom_v, conform_task, conform_detail, request);
        },

        [=](conform_atom,
            const std::string &conform_task,
            const utility::JsonStore &conform_detail,
            const UuidActor &playlist,
            const UuidActorVector &media,
            const UuidVector &insert_before) {
            delegate(
                pool_,
                conform_atom_v,
                conform_task,
                conform_detail,
                playlist,
                media,
                insert_before);
        },

        [=](conform_tasks_atom) -> result<std::vector<std::string>> {
            auto rp = make_response_promise<std::vector<std::string>>();

            request(pool_, infinite, conform_tasks_atom_v)
                .then(
                    [=](const std::vector<std::string> &result) mutable {
                        // compare with model and replace as required.
                        // simple purge..
                        try {
                            if (data_.at("children").size())
                                data_.remove_rows(0, data_.at("children").size(), "");

                            if (not result.empty()) {
                                auto jsn = R"([])"_json;
                                for (const auto &i : result) {
                                    auto item    = R"({"name":null})"_json;
                                    item["name"] = i;
                                    jsn.push_back(item);
                                }

                                data_.insert_rows(0, jsn.size(), jsn, "");
                            }

                            send(
                                event_group_,
                                utility::event_atom_v,
                                conform_tasks_atom_v,
                                result);

                            rp.deliver(result);
                        } catch (const std::exception &err) {
                            rp.deliver(make_error(sec::runtime_error, err.what()));
                        }
                    },
                    [=](const error &err) mutable { rp.deliver(err); });

            return rp;
        },

        [=](json_store::update_atom,
            const JsonStore & /*change*/,
            const std::string & /*path*/,
            const JsonStore &full) {
            delegate(actor_cast<caf::actor>(this), json_store::update_atom_v, full);
        },

        [=](json_store::update_atom, const JsonStore &j) mutable {
            try {
                auto count = preference_value<size_t>(j, "/core/conform/max_worker_count");
                if (count > worker_count_) {
                    spdlog::debug(
                        "conform workers changed old {} new {}", worker_count_, count);
                    while (worker_count_ < count) {
                        anon_send(
                            pool_,
                            sys_atom_v,
                            put_atom_v,
                            system().spawn<ConformWorkerActor>());
                        worker_count_++;
                    }
                } else if (count < worker_count_) {
                    spdlog::debug(
                        "conform workers changed old {} new {}", worker_count_, count);
                    // get actors..
                    worker_count_ = count;
                    request(pool_, infinite, sys_atom_v, get_atom_v)
                        .await(
                            [=](const std::vector<actor> &ws) {
                                for (auto i = worker_count_; i < ws.size(); i++) {
                                    anon_send(pool_, sys_atom_v, delete_atom_v, ws[i]);
                                }
                            },
                            [=](const error &err) {
                                throw std::runtime_error(
                                    "Failed to find pool " + to_string(err));
                            });
                }
            } catch (...) {
            }
        });
}


void ConformManagerActor::on_exit() { system().registry().erase(conform_registry); }
