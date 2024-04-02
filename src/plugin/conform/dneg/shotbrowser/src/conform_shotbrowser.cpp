// SPDX-License-Identifier: Apache-2.0

#include "xstudio/conform/conformer.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/string_helpers.hpp"
#include "xstudio/utility/json_store.hpp"
#include "xstudio/utility/json_store_sync.hpp"

#include "../../../../data_source/dneg/shotbrowser/src/query_engine.hpp"

using namespace xstudio;
using namespace xstudio::conform;
using namespace xstudio::utility;

class ShotbrowserConform : public Conformer {
  public:
    ShotbrowserConform(const utility::JsonStore &prefs = utility::JsonStore())
        : Conformer(prefs) {}
    ~ShotbrowserConform() = default;

    std::vector<std::string> conform_tasks() override { return tasks_; }

    bool update_tasks(const utility::JsonStoreSync &presets) {
        auto result = false;
        std::vector<std::string> tasks;
        std::map<std::string, utility::Uuid> task_uuids;

        // find entry with uuid  == b6e0ca0e-2d23-4b1d-a33a-761596183d5f
        try {
            auto task_group =
                presets.find_first("id", Uuid("b6e0ca0e-2d23-4b1d-a33a-761596183d5f"));
            if (task_group) {
                // collect preset names.
                for (const auto &i :
                     presets.at(*task_group).at("children").at(1).at("children")) {

                    tasks.emplace_back(i.value("name", ""));
                    task_uuids[i.value("name", "")] = i.value("id", utility::Uuid());
                }
            }
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }

        if (tasks != tasks_) {
            tasks_      = tasks;
            task_uuids_ = task_uuids;
            result      = true;
        }

        return result;
    }

    std::optional<utility::Uuid> get_task_id(const std::string &name) const {
        if (task_uuids_.count(name))
            return task_uuids_.at(name);

        return {};
    }

  private:
    std::vector<std::string> tasks_;
    std::map<std::string, utility::Uuid> task_uuids_;
};

template <typename T> class ShotbrowserConformActor : public caf::event_based_actor {
  public:
    ShotbrowserConformActor(
        caf::actor_config &cfg, const utility::JsonStore &prefs = utility::JsonStore())
        : caf::event_based_actor(cfg), conform_(prefs) {
        spdlog::debug("Created ShotbrowserConformActor");
        utility::print_on_exit(this, "ShotbrowserConformActor");

        {
            auto prefs = global_store::GlobalStoreHelper(system());
            utility::JsonStore js;
            utility::join_broadcast(this, prefs.get_group(js));
            conform_.update_preferences(js);
        }

        // we need to subscribe to the shotbrowsers preset model


        behavior_.assign(
            [=](xstudio::broadcast::broadcast_down_atom, const caf::actor_addr &) {},

            [=](conform_atom,
                const std::string &conform_task,
                const utility::JsonStore &conform_detail,
                const ConformRequest &crequest) -> result<ConformReply> {
                auto rp = make_response_promise<ConformReply>();

                try {

                    if (not connected_) {
                        setup();
                        conform_.update_tasks(presets_);
                    }

                    // spdlog::warn("conform_request {} {}", conform_task,
                    // conform_detail.dump(2)); spdlog::warn("conform_request {}",
                    // crequest.playlist_json_.dump(2));

                    auto query_id = conform_.get_task_id(conform_task);

                    if (query_id) {

                        // for (const auto &i : crequest.items_) {
                        //     spdlog::warn("conform_request {}", std::get<0>(i).dump(2));
                        // }

                        if (crequest.items_.size() != 1) {
                            spdlog::warn("NOT SUPPORTED YET");
                            rp.deliver(ConformReply());
                            return rp;
                        }

                        // build a query....
                        auto shotbrowser =
                            system().registry().template get<caf::actor>("SHOTBROWSER");
                        if (shotbrowser) {
                            auto metadata = std::get<1>(crequest.items_.at(0));

                            auto project_id =
                                QueryEngine::get_project_id(metadata, JsonStore());
                            if (not project_id)
                                throw std::runtime_error("Failed to find project_id");
                            // here we go....

                            auto req          = JsonStore(GetExecutePreset);
                            req["project_id"] = project_id;
                            req["preset_id"]  = *query_id;
                            req["metadata"]   = metadata;
                            req["context"]    = R"({
                                "type": null,
                                "epoc": null,
                                "audio_source": "",
                                "visual_source": "",
                                "flag_text": "",
                                "flag_colour": "",
                                "truncated": false
                            })"_json;

                            // req["env"]          = qvariant_to_json(env);
                            // req["custom_terms"] = qvariant_to_json(custom_terms);

                            // req["context"]["epoc"] =
                            // utility::to_epoc_milliseconds(utility::clock::now());
                            // req["context"]["type"] = "result";

                            request(shotbrowser, infinite, data_source::get_data_atom_v, req)
                                .then(
                                    [=](const JsonStore &result) mutable {
                                        spdlog::warn("{}", result.dump(2));

                                        // we need to create media from result..
                                        // there maybe multiple items in the result.
                                        request(
                                            shotbrowser,
                                            infinite,
                                            playlist::add_media_atom_v,
                                            result,
                                            crequest.playlist_.uuid(),
                                            crequest.playlist_.actor(),
                                            std::get<2>(crequest.items_.at(0)))
                                            .then(
                                                [=](const UuidActorVector &new_media) mutable {
                                                    auto creply = ConformReply();
                                                    auto ritems =
                                                        std::vector<ConformReplyItem>();

                                                    for (const auto &i : new_media) {
                                                        ritems.emplace_back(std::make_tuple(i));
                                                    }
                                                    creply.items_.push_back(ritems);

                                                    rp.deliver(creply);
                                                },
                                                [=](caf::error &err) mutable {
                                                    rp.deliver(err);
                                                });
                                    },
                                    [=](caf::error &err) mutable { rp.deliver(err); });
                        } else {
                            throw std::runtime_error("Failed to find shotbrowser");
                        }
                    } else {
                        throw std::runtime_error("Failed to find query id");
                    }
                } catch (const std::exception &err) {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    rp.deliver(make_error(xstudio_error::error, err.what()));
                }

                return rp;
            },

            [=](conform_tasks_atom) -> std::vector<std::string> {
                setup();
                conform_.update_tasks(presets_);
                // for(const auto &i: conform_.conform_tasks())
                //     spdlog::warn("conform_tasks_atom {}",i);
                return conform_.conform_tasks();
            },

            [=](utility::event_atom,
                json_store::sync_atom,
                const Uuid &uuid,
                const JsonStore &event) {
                if (uuid == user_preset_event_id_) {
                    presets_.process_event(event);
                    if (conform_.update_tasks(presets_))
                        anon_send(
                            system().registry().template get<caf::actor>(conform_registry),
                            conform_tasks_atom_v);
                }
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

    void setup() {
        if (not connected_) {
            sb_actor_ = system().registry().template get<caf::actor>("SHOTBROWSER");
            if (sb_actor_) {
                scoped_actor sys{system()};
                try {
                    auto uuids =
                        request_receive<UuidVector>(*sys, sb_actor_, json_store::sync_atom_v);
                    user_preset_event_id_ = uuids[0];

                    // get system presets
                    auto data = request_receive<JsonStore>(
                        *sys, sb_actor_, json_store::sync_atom_v, user_preset_event_id_);
                    presets_ = JsonStoreSync(data);


                    // join events.
                    if (preset_events_) {
                        try {
                            request_receive<bool>(
                                *sys, preset_events_, broadcast::leave_broadcast_atom_v, this);
                        } catch (const std::exception &) {
                        }
                        preset_events_ = caf::actor();
                    }
                    try {
                        preset_events_ = request_receive<caf::actor>(
                            *sys, sb_actor_, get_event_group_atom_v);
                        request_receive<bool>(
                            *sys, preset_events_, broadcast::join_broadcast_atom_v, this);
                    } catch (const std::exception &err) {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    }
                    connected_ = true;
                } catch (const std::exception &err) {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                }
            } else {
                // spdlog::warn("NOT CONNECTED");
            }
        }
    }

    ~ShotbrowserConformActor() override = default;
    caf::behavior make_behavior() override { return behavior_; }

  private:
    caf::behavior behavior_;
    T conform_;
    utility::Uuid user_preset_event_id_;
    utility::JsonStoreSync presets_;
    caf::actor sb_actor_;
    caf::actor preset_events_;
    bool connected_{false};
};

extern "C" {
plugin_manager::PluginFactoryCollection *plugin_factory_collection_ptr() {
    return new plugin_manager::PluginFactoryCollection(
        std::vector<std::shared_ptr<plugin_manager::PluginFactory>>(
            {std::make_shared<ConformPlugin<ShotbrowserConformActor<ShotbrowserConform>>>(
                Uuid("ebeecb15-75c0-4aa2-9cc7-1b3ad2491c39"),
                "DNeg",
                "DNeg",
                "DNeg Shotbrowser Conformer",
                semver::version("1.0.0"))}));
}
}
