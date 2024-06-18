// SPDX-License-Identifier: Apache-2.0

#include "xstudio/conform/conformer.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/string_helpers.hpp"
#include "xstudio/utility/json_store.hpp"
#include "xstudio/timeline/track_actor.hpp"
#include "xstudio/utility/json_store_sync.hpp"

#include "../../../../data_source/dneg/shotbrowser/src/query_engine.hpp"

using namespace xstudio;
using namespace xstudio::conform;
using namespace xstudio::utility;
using namespace std::chrono_literals;

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
                    if (i.value("hidden", false))
                        continue;

                    if (not i.value("favourite", true))
                        continue;

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

    std::optional<utility::Uuid> get_cut_query_id(const utility::JsonStoreSync &presets) const {
        try {
            auto task_group =
                presets.find_first("id", Uuid("86439af3-16be-4a2f-89f3-ee1e5810ae47"));
            if (task_group) {
                // collect preset names.
                for (const auto &i :
                     presets.at(*task_group).at("children").at(1).at("children")) {
                    if (i.value("hidden", false))
                        continue;

                    if (not i.value("favourite", true))
                        continue;

                    return i.value("id", utility::Uuid());
                }
            }
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }

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

            // conform media into clips, doesn't require shot browser.. as we only key off
            // show/shot.
            [=](conform_atom, const ConformRequest &crequest) -> result<ConformReply> {
                auto rp = make_response_promise<ConformReply>();
                try {
                    auto creply = ConformReply(crequest);
                    auto clips =
                        crequest.template_tracks_.at(0).find_all_items(timeline::IT_CLIP);

                    // build clip lookup.
                    std::map<Uuid, std::string> clip_project_map;
                    std::map<Uuid, std::string> clip_shot_map;

                    // remap start frame if requested via metadata..
                    for (auto &t : creply.request_.template_tracks_) {
                        for (auto &c : t.find_all_items(timeline::IT_CLIP)) {

                            auto item_meta = c.get().prop();
                            const auto cut_start_ptr =
                                json::json_pointer("/metadata/external/DNeg/cut/start");
                            const auto override_cut_ptr =
                                json::json_pointer("/metadata/external/DNeg/cut/override");
                            const auto override_comp_ptr =
                                json::json_pointer("/metadata/external/DNeg/comp/override");

                            if (item_meta.contains(override_cut_ptr) and
                                item_meta.at(override_cut_ptr).get<bool>() and
                                item_meta.contains(cut_start_ptr) and
                                not item_meta.at(cut_start_ptr).is_null()) {
                                // get item trimmed start frame
                                auto trimmed_range = c.get().trimmed_range();

                                item_meta.at(override_cut_ptr)  = false;
                                item_meta.at(override_comp_ptr) = false;
                                trimmed_range.set_start(FrameRate(
                                    trimmed_range.rate() *
                                    item_meta.at(cut_start_ptr).get<int>()));

                                c.get().set_prop(item_meta);
                                c.get().set_active_range(trimmed_range);
                            }
                        }
                    }


                    for (const auto &c : clips) {
                        auto clip_uuid  = c.get().uuid();
                        auto media_uuid = c.get().prop().value("media_uuid", Uuid());

                        auto clip_project =
                            QueryEngine::get_project_name(crequest.metadata_.at(clip_uuid));
                        auto clip_shot =
                            QueryEngine::get_shot_name(crequest.metadata_.at(clip_uuid));

                        if (clip_project.empty() and not media_uuid.is_null())
                            clip_project = QueryEngine::get_project_name(
                                crequest.metadata_.at(media_uuid));
                        if (clip_shot.empty() and not media_uuid.is_null())
                            clip_shot =
                                QueryEngine::get_shot_name(crequest.metadata_.at(media_uuid));

                        clip_project_map[clip_uuid] = clip_project;
                        clip_shot_map[clip_uuid]    = clip_shot;

                        if (clip_project.empty() or clip_shot.empty()) {
                            spdlog::warn(
                                "Clip metadata not found, {} project: '{}', 'shot':  {}",
                                c.get().name(),
                                clip_project,
                                clip_shot);
                        }
                    }

                    std::set<utility::Uuid> matched_clips;
                    const auto only_one_match =
                        crequest.operations_.value("only_one_clip_match", false);

                    auto clip_track_uuid = Uuid();

                    // we're matching media to clips.
                    for (const auto &i : crequest.items_) {
                        // find match in clips..
                        // get show shot..
                        if (clip_track_uuid != i.clip_track_uuid_) {
                            matched_clips.clear();
                            clip_track_uuid = i.clip_track_uuid_;
                        }

                        if (clips.empty()) {
                            spdlog::warn("No clips found on selected conform track.");
                            creply.items_.push_back({});
                        } else {
                            try {
                                // spdlog::warn("GET MEDIA SHOW/SHOT {} {}",
                                // to_string(std::get<0>(i).uuid()),
                                // crequest.metadata_.count(std::get<0>(i).uuid()));
                                const auto meta = crequest.metadata_.at(i.item_.uuid());
                                auto project    = QueryEngine::get_project_name(meta);
                                auto shot       = QueryEngine::get_shot_name(meta);

                                if (project.empty() or shot.empty()) {
                                    creply.items_.push_back({});
                                    spdlog::warn(
                                        "Media is missing metadata, {} project: {}, shot: {}",
                                        to_string(i.item_.uuid()),
                                        project,
                                        shot);
                                } else {
                                    auto ritems = std::vector<ConformReplyItem>();

                                    for (const auto &c : clips) {
                                        auto clip_uuid = c.get().uuid();
                                        if ((not only_one_match or
                                             not matched_clips.count(clip_uuid)) and
                                            clip_project_map.at(clip_uuid) == project and
                                            clip_shot_map.at(clip_uuid) == shot) {
                                            ritems.push_back(
                                                std::make_tuple(c.get().uuid_actor()));
                                            matched_clips.insert(clip_uuid);
                                            if (only_one_match)
                                                break;
                                        }
                                    }
                                    if (ritems.empty()) {
                                        spdlog::warn(
                                            "Media has no matching clip {} project: {}, shot:  "
                                            "{}",
                                            to_string(i.item_.uuid()),
                                            project,
                                            shot);
                                        creply.items_.push_back({});
                                    } else
                                        creply.items_.push_back(ritems);
                                }

                            } catch (const std::exception &err) {
                                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                                creply.items_.push_back({});
                            }
                        }
                    }

                    rp.deliver(creply);
                } catch (const std::exception &err) {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    rp.deliver(make_error(xstudio_error::error, err.what()));
                }

                return rp;
            },

            [=](conform_atom,
                const std::string &conform_task,
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
                        // build a query....
                        if (crequest.items_.empty()) {
                            rp.deliver(ConformReply(crequest));
                            return rp;
                        }

                        auto shotbrowser =
                            system().registry().template get<caf::actor>("SHOTBROWSER");

                        if (not shotbrowser)
                            throw std::runtime_error("Failed to find shotbrowser");


                        auto shotgrid_count = std::make_shared<size_t>(crequest.items_.size());
                        auto shotgrid_results = std::make_shared<std::vector<UuidActorVector>>(
                            crequest.items_.size());

                        // dispatch requests for shotgrid data.
                        for (size_t i = 0; i < crequest.items_.size(); i++) {
                            auto metadata =
                                crequest.metadata_.at(crequest.items_.at(i).item_.uuid());

                            // clip metadata has precedence
                            auto media_uuid = metadata.value("media_uuid", Uuid());
                            if (not media_uuid.is_null() and
                                crequest.metadata_.count(media_uuid)) {
                                auto tmp = crequest.metadata_.at(media_uuid);
                                tmp.update(metadata);
                                metadata = tmp;
                                // metadata.update(crequest.metadata_.at(media_uuid));
                            }

                            auto project_id =
                                QueryEngine::get_project_id(metadata, JsonStore());

                            // if (not project_id)
                            //     throw std::runtime_error("Failed to find project_id");
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
                                        request(
                                            shotbrowser,
                                            infinite,
                                            playlist::add_media_atom_v,
                                            result,
                                            crequest.container_.uuid(),
                                            crequest.container_.actor(),
                                            crequest.items_.at(i).before_)
                                            .then(
                                                [=](const UuidActorVector &new_media) mutable {
                                                    (*shotgrid_results)[i] = new_media;
                                                    (*shotgrid_count)--;
                                                    if (not *shotgrid_count)
                                                        process_results(
                                                            rp, *shotgrid_results, crequest);
                                                },
                                                [=](caf::error &err) mutable {
                                                    (*shotgrid_count)--;
                                                    if (not *shotgrid_count)
                                                        process_results(
                                                            rp, *shotgrid_results, crequest);
                                                });
                                    },
                                    [=](caf::error &err) mutable {
                                        (*shotgrid_count)--;
                                        if (not *shotgrid_count)
                                            process_results(rp, *shotgrid_results, crequest);
                                    });
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

            [=](conform_atom,
                const UuidActor &timeline,
                const bool only_create_conform_track) -> result<bool> {
                // get timeline detail.
                auto rp = make_response_promise<bool>();

                scoped_actor sys{system()};
                try {
                    auto found_project = std::string();
                    auto timeline_item = request_receive<timeline::Item>(
                        *sys, timeline.actor(), timeline::item_atom_v);


                    auto timeline_path = timeline_item.prop().value("path", std::string());
                    if (not timeline_path.empty()) {
                        std::cmatch m;
                        auto uri_path         = caf::make_uri(timeline_path);
                        const auto SHOW_REGEX = std::regex(
                            R"(^(?:/jobs|/hosts/[^/]+/user_data\d*)/([A-Z0-9]+)/.+$)");
                        if (uri_path and
                            std::regex_match(
                                uri_to_posix_path(*uri_path).c_str(), m, SHOW_REGEX)) {
                            found_project = m[1];
                        }
                    }
                    // process timeline..
                    // purge empty tracks.

                    auto video_tracks = timeline_item.find_all_items(timeline::IT_VIDEO_TRACK);
                    auto insert_index = static_cast<int>(video_tracks.size());
                    auto vcount       = video_tracks.size();
                    if (not only_create_conform_track) {
                        for (auto &i : video_tracks) {
                            if (i.get().empty() and vcount > 1) {
                                auto pactor = find_parent_actor(timeline_item, i.get().uuid());
                                if (pactor) {
                                    insert_index--;
                                    vcount--;
                                    request_receive<JsonStore>(
                                        *sys,
                                        pactor,
                                        timeline::erase_item_atom_v,
                                        i.get().uuid(),
                                        true);
                                }
                            }
                        }

                        auto audio_tracks =
                            timeline_item.find_all_items(timeline::IT_AUDIO_TRACK);
                        auto acount = audio_tracks.size();
                        for (auto &i : audio_tracks) {
                            if (i.get().empty() and acount > 1) {
                                auto pactor = find_parent_actor(timeline_item, i.get().uuid());
                                if (pactor) {
                                    acount--;
                                    request_receive<JsonStore>(
                                        *sys,
                                        pactor,
                                        timeline::erase_item_atom_v,
                                        i.get().uuid(),
                                        true);
                                }
                            }
                        }
                    }
                    // create a new track with empty clips based off markers and scan track..
                    // populate clips with metadata required to conform timeline
                    auto vtrack = timeline::Item(timeline::IT_NONE);
                    std::reverse(video_tracks.begin(), video_tracks.end());

                    for (const auto &i : video_tracks) {
                        if (not i.get().empty()) {
                            vtrack = i.get();
                            break;
                        }
                    }

                    vtrack.set_name("Conform Track");
                    vtrack.set_locked(true);
                    vtrack.set_enabled(false);
                    // populate vtrack name/metadata
                    if (not only_create_conform_track)
                        anon_send(vtrack.actor(), timeline::item_lock_atom_v, true);

                    auto media_metadata = std::map<Uuid, JsonStore>();
                    auto tframe         = timeline_item.trimmed_start();
                    const auto trate    = timeline_item.rate();

                    for (auto &i : vtrack.children()) {
                        if (i.item_type() == timeline::IT_CLIP) {
                            auto check_clips = true;
                            i.set_name("UNKNOWN");

                            // mirror changes to Cut Ref Track
                            if (not only_create_conform_track)
                                anon_send(i.actor(), timeline::item_name_atom_v, "UNKNOWN");

                            // leed a list of clips at this point in time backed down.
                            // do we need to check markers..
                            if (not found_project.empty()) {
                                // marker should have same start time as clip..
                                // markers exist on stack..
                                const auto fcpp = json::json_pointer("/fcp_xml/comment");
                                const static auto cutcompre =
                                    std::regex("(\\d+),(\\d+)-(\\d+),(\\d+)");

                                for (const auto &m :
                                     timeline_item.children().front().markers()) {
                                    if (m.start() == tframe) {
                                        auto u =
                                            R"({"metadata": {"external": {"DNeg": {"shot": null, "show":null, "comp": {"start": null, "end":null, "override":true}, "cut": {"start": null, "end":null, "override":true}}}}})"_json;

                                        if (m.prop().contains(fcpp)) {
                                            auto comment = m.prop().at(fcpp).get<std::string>();
                                            std::cmatch match;
                                            if (std::regex_match(
                                                    comment.c_str(), match, cutcompre)) {
                                                auto start_frame = std::stoi(match[2]);
                                                i.set_active_range(FrameRange(
                                                    FrameRate(start_frame * trate.to_flicks()),
                                                    i.trimmed_duration(),
                                                    i.rate()));
                                                i.set_available_range(*i.active_range());
                                                check_clips = false;

                                                u[json::json_pointer(
                                                    "/metadata/external/DNeg/comp/start")] =
                                                    std::stoi(match[1]);
                                                u[json::json_pointer(
                                                    "/metadata/external/DNeg/cut/start")] =
                                                    std::stoi(match[2]);
                                                u[json::json_pointer(
                                                    "/metadata/external/DNeg/cut/end")] =
                                                    std::stoi(match[3]);
                                                u[json::json_pointer(
                                                    "/metadata/external/DNeg/comp/end")] =
                                                    std::stoi(match[4]);
                                            }
                                        }

                                        auto meta = i.prop();
                                        u[json::json_pointer("/metadata/external/DNeg/shot")] =
                                            m.name();
                                        u[json::json_pointer("/metadata/external/DNeg/show")] =
                                            found_project;
                                        meta.update(u);

                                        i.set_prop(meta);
                                        i.set_name(m.name());

                                        if (not only_create_conform_track) {
                                            anon_send(
                                                i.actor(), timeline::item_prop_atom_v, meta);
                                            anon_send(
                                                i.actor(),
                                                timeline::item_name_atom_v,
                                                m.name());
                                        }
                                        break;
                                    }
                                }
                            }

                            if (check_clips) {
                                auto items = timeline_item.resolve_time_raw(tframe);

                                for (const auto &j : items) {
                                    auto clip = j.first;

                                    try {
                                        auto project =
                                            QueryEngine::get_project_name(clip.prop());
                                        auto shot =
                                            QueryEngine::get_shot_name(clip.prop(), true);

                                        if (project.empty() or shot.empty()) {
                                            // try media metadata..
                                            auto media_uuid =
                                                clip.prop().value("media_uuid", Uuid());

                                            if (not media_uuid.is_null() and
                                                not media_metadata.count(media_uuid)) {
                                                try {
                                                    auto tries    = 10;
                                                    auto metadata = JsonStore();
                                                    while (true) {
                                                        metadata = request_receive<JsonStore>(
                                                            *sys,
                                                            clip.actor(),
                                                            playlist::get_media_atom_v,
                                                            json_store::get_json_atom_v,
                                                            Uuid(),
                                                            "");
                                                        tries--;
                                                        if (not tries or not metadata.is_null())
                                                            break;
                                                        std::this_thread::sleep_for(200ms);
                                                        // spdlog::warn("retry");
                                                    }

                                                    if (metadata.is_null())
                                                        spdlog::warn(
                                                            "No metadata {} {} {}",
                                                            clip.name(),
                                                            metadata.dump(2),
                                                            tries);

                                                    media_metadata[media_uuid] = metadata;
                                                } catch (const std::exception &err) {
                                                    spdlog::warn(
                                                        "{} {} {}",
                                                        __PRETTY_FUNCTION__,
                                                        clip.name(),
                                                        err.what());
                                                    media_metadata[media_uuid] = R"({})"_json;
                                                }
                                            }

                                            if (project.empty())
                                                project = QueryEngine::get_project_name(
                                                    media_metadata.at(media_uuid));
                                            if (shot.empty())
                                                shot = QueryEngine::get_shot_name(
                                                    media_metadata.at(media_uuid), true);
                                        }

                                        if (not project.empty())
                                            found_project = project;

                                        if (not project.empty() and not shot.empty()) {
                                            auto m = i.prop();
                                            auto u =
                                                R"({"metadata": {"external": {"DNeg": {"shot": null, "show":null, "comp": {"start": null, "end":null, "override":false}, "cut": {"start": null, "end":null, "override":true}}}}})"_json;

                                            u[json::json_pointer(
                                                "/metadata/external/DNeg/shot")] = shot;
                                            u[json::json_pointer(
                                                "/metadata/external/DNeg/show")] = project;

                                            u[json::json_pointer(
                                                "/metadata/external/DNeg/cut/start")] =
                                                clip.trimmed_frame_start().frames();
                                            u[json::json_pointer(
                                                "/metadata/external/DNeg/cut/end")] =
                                                clip.trimmed_frame_start().frames() +
                                                clip.trimmed_frame_duration().frames() - 1;

                                            m.update(u);
                                            i.set_prop(m);
                                            i.set_name(shot);
                                            // if(clip.available_range()) {
                                            //     // must not be smaller than current active
                                            //     i.set_available_range(*(clip.available_range()));
                                            // }
                                            i.set_active_range(FrameRange(
                                                clip.trimmed_start(),
                                                i.trimmed_duration(),
                                                i.rate()));
                                            i.set_available_range(*i.active_range());
                                            // always use markers...
                                            // check_markers = false;

                                            if (not only_create_conform_track) {
                                                anon_send(
                                                    i.actor(), timeline::item_prop_atom_v, m);
                                                anon_send(
                                                    i.actor(),
                                                    timeline::item_name_atom_v,
                                                    shot);
                                            }

                                            break;
                                        }

                                    } catch (const std::exception &err) {
                                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                                    }
                                }
                            }
                        }
                        if (i.name() == "UNKNOWN") {
                            if (not only_create_conform_track)
                                anon_send(i.actor(), timeline::item_flag_atom_v, "#FFFF0000");

                            i.set_flag("#FFFF0000");
                            i.set_enabled(false);
                        } else {
                            // mirror changes to Cut Ref Track
                            if (not only_create_conform_track)
                                anon_send(i.actor(), timeline::item_flag_atom_v, "#FF00FF00");

                            i.set_flag("#FF00FF00");
                        }
                        tframe += i.trimmed_duration();
                    }


                    if (only_create_conform_track) {
                        // clean before adding
                        vtrack.reset_uuid(true);
                        vtrack.reset_actor(true);
                        vtrack.reset_media_uuid();
                        auto vua =
                            UuidActor(vtrack.uuid(), spawn<timeline::TrackActor>(vtrack));

                        request_receive<JsonStore>(
                            *sys,
                            timeline_item.children().front().actor(),
                            timeline::insert_item_atom_v,
                            insert_index,
                            UuidActorVector({vua}));
                    }

                    auto tprop                  = timeline_item.prop();
                    tprop["conform_track_uuid"] = vtrack.uuid();
                    request_receive<JsonStore>(
                        *sys, timeline_item.actor(), timeline::item_prop_atom_v, tprop);

                    // purge other video tracks
                    if (not only_create_conform_track and vcount > 1) {

                        request_receive<JsonStore>(
                            *sys,
                            timeline_item.children().front().actor(),
                            timeline::erase_item_atom_v,
                            0,
                            static_cast<int>(vcount - 1),
                            true);
                    }

                    rp.deliver(true);
                } catch (const std::exception &err) {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    rp.deliver(false);
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

            [=](conform_atom,
                const std::vector<std::pair<utility::UuidActor, utility::JsonStore>> &media)
                -> result<std::vector<std::optional<std::pair<std::string, caf::uri>>>> {
                // build list of sequence id's
                auto rp = make_response_promise<
                    std::vector<std::optional<std::pair<std::string, caf::uri>>>>();

                // for(const auto &i: media) {
                //     spdlog::warn("{}", i.second.dump(2));
                // }

                try {

                    if (not connected_) {
                        setup();
                        conform_.update_tasks(presets_);
                    }

                    auto query_id = conform_.get_cut_query_id(presets_);

                    auto shotbrowser =
                        system().registry().template get<caf::actor>("SHOTBROWSER");

                    if (not shotbrowser)
                        throw std::runtime_error("Failed to find shotbrowser");

                    if (query_id) {

                        auto shotgrid_count   = std::make_shared<size_t>(media.size());
                        auto shotgrid_results = std::make_shared<
                            std::vector<std::optional<std::pair<std::string, caf::uri>>>>(
                            media.size());

                        auto count = 0;
                        for (const auto &i : media) {
                            auto project_id =
                                QueryEngine::get_project_id(i.second, JsonStore());

                            auto req          = JsonStore(GetExecutePreset);
                            req["project_id"] = project_id;
                            req["preset_id"]  = *query_id;
                            req["metadata"]   = i.second;
                            req["context"]    = R"({
                                "type": null,
                                "epoc": null,
                                "audio_source": "",
                                "visual_source": "",
                                "flag_text": "",
                                "flag_colour": "",
                                "truncated": false
                            })"_json;

                            request(shotbrowser, infinite, data_source::get_data_atom_v, req)
                                .then(
                                    [=](const JsonStore &result) mutable {
                                        auto jpath = nlohmann::json::json_pointer(
                                            "/result/data/0/attributes");
                                        auto spath = nlohmann::json::json_pointer(
                                            "/result/data/0/relationships/entity/data");
                                        if (result.contains(jpath)) {

                                            auto fspath = fs::path(result.at(jpath).value(
                                                "sg_path_to_frames", std::string()));
                                            // no idea why we have to do parent path twice.. BUG
                                            // ?
                                            auto otiopath =
                                                fspath.parent_path().parent_path() /
                                                (fspath.stem().string() + std::string(".otio"));
                                            auto name =
                                                result.at(spath).value("name", std::string());

                                            try {
                                                auto uri = posix_path_to_uri(otiopath.string());
                                                (*shotgrid_results)[count] =
                                                    std::make_pair(name, uri);
                                            } catch (...) {
                                                (*shotgrid_results)[count] = {};
                                            }
                                        } else
                                            (*shotgrid_results)[count] = {};

                                        // spdlog::warn("{}",result.dump(2));

                                        (*shotgrid_count)--;
                                        if (not *shotgrid_count)
                                            rp.deliver(*shotgrid_results);
                                    },
                                    [=](caf::error &err) mutable {
                                        (*shotgrid_count)--;
                                        if (not *shotgrid_count)
                                            rp.deliver(*shotgrid_results);
                                    });
                            count++;
                        }
                    } else {
                        spdlog::warn("{} Sequence preset not found", __PRETTY_FUNCTION__);
                        rp.deliver(std::vector<std::optional<std::pair<std::string, caf::uri>>>(
                            media.size()));
                    }
                } catch (const std::exception &err) {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    rp.deliver(make_error(xstudio_error::error, err.what()));
                }

                return rp;
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

    void process_results(
        caf::typed_response_promise<ConformReply> rp,
        const std::vector<UuidActorVector> &results,
        const ConformRequest &crequest) {
        auto creply = ConformReply(crequest);

        // adjust the clip start frame if it doesn't match the clip metadata
        // to support using the client movie as the conform track, which will have useless
        // trimmed clip start frames. item_ should be a clone ?
        if (crequest.item_type_ == "Clip") {
            scoped_actor sys{system()};

            for (auto &i : creply.request_.items_) {
                // check item_metadata
                const auto &item_meta = creply.request_.metadata_.at(i.item_.uuid());
                const auto cut_start_ptr =
                    json::json_pointer("/metadata/external/DNeg/cut/start");
                const auto override_cut_ptr =
                    json::json_pointer("/metadata/external/DNeg/cut/override");
                const auto override_comp_ptr =
                    json::json_pointer("/metadata/external/DNeg/comp/override");

                if (item_meta.contains(override_cut_ptr) and
                    item_meta.at(override_cut_ptr).get<bool>() and
                    item_meta.contains(cut_start_ptr) and
                    not item_meta.at(cut_start_ptr).is_null()) {
                    // get item trimmed start frame
                    auto prop = request_receive<JsonStore>(
                        *sys, i.item_.actor(), timeline::item_prop_atom_v);
                    auto trimmed_range = request_receive<FrameRange>(
                        *sys, i.item_.actor(), timeline::trimmed_range_atom_v);

                    prop.at(override_cut_ptr)  = false;
                    prop.at(override_comp_ptr) = false;
                    trimmed_range.set_start(FrameRate(
                        trimmed_range.rate() * item_meta.at(cut_start_ptr).get<int>()));

                    anon_send(i.item_.actor(), timeline::item_prop_atom_v, prop);
                    anon_send(i.item_.actor(), timeline::active_range_atom_v, trimmed_range);
                }

                // spdlog::warn("{}", creply.request_.metadata_.at(i.item_.uuid()).dump(2));
            }
        }

        for (const auto &i : results) {
            auto ritems = std::vector<ConformReplyItem>();

            for (const auto &j : i)
                ritems.emplace_back(std::make_tuple(j));

            creply.items_.push_back(ritems);
        }

        creply.operations_["create_media"] = true;
        creply.operations_["insert_media"] = true;

        rp.deliver(creply);
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
