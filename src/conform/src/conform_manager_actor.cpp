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
#include "xstudio/timeline/item.hpp"
#include "xstudio/timeline/track_actor.hpp"
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
            const utility::JsonStore &conform_operations,
            const UuidActor &playlist,
            const UuidActor &timeline,
            const UuidActor &conform_track,
            const UuidActorVector &media) -> result<ConformReply> {
            auto rp = make_response_promise<ConformReply>();

            // get copy of conform track.
            try {
                scoped_actor sys{system()};

                auto timeline_prop = request_receive<JsonStore>(
                    *sys, timeline.actor(), timeline::item_prop_atom_v);

                auto conform_track_uuid = Uuid();

                if (conform_track.uuid().is_null()) {
                    if (timeline_prop.is_null() or
                        not timeline_prop.count("conform_track_uuid"))
                        throw std::runtime_error("No conform track defined.");
                    conform_track_uuid =
                        timeline_prop.value("conform_track_uuid", utility::Uuid());
                } else {
                    conform_track_uuid = conform_track.uuid();
                }

                if (conform_track_uuid.is_null())
                    throw std::runtime_error("No conform track defined.");

                // find track..
                auto track_item = request_receive<timeline::Item>(
                    *sys, timeline.actor(), timeline::item_atom_v, conform_track_uuid);

                track_item.unbind();

                // need media metadata populating.
                auto ritems = std::vector<ConformRequestItem>();
                for (const auto &i : media) {
                    ritems.emplace_back(std::make_tuple(i, i, Uuid()));
                }

                auto crequest        = ConformRequest(playlist, timeline, track_item, ritems);
                crequest.operations_ = conform_operations;

                auto clip_items = track_item.find_all_items(timeline::IT_CLIP);
                for (const auto &i : clip_items)
                    crequest.metadata_[i.get().uuid()] = i.get().prop();

                // need to populate all metadata.
                conform_chain(rp, crequest);

            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                rp.deliver(make_error(sec::runtime_error, err.what()));
            }

            return rp;
        },

        [=](conform_atom, const UuidActor &timeline) -> result<bool> {
            // make worker gather all the information
            auto rp = make_response_promise<bool>();
            try {
                if (not conformers.empty()) {
                    // request.dump();
                    fan_out_request<policy::select_all>(
                        conformers, infinite, conform_atom_v, timeline)
                        .then(
                            [=](const std::vector<bool> all_results) mutable {
                                // compile results..
                                auto result = false;

                                for (const auto &i : all_results)
                                    result |= i;
                                rp.deliver(result);
                            },
                            [=](const error &err) mutable { rp.deliver(err); });
                } else {
                    rp.deliver(true);
                }
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                rp.deliver(make_error(sec::runtime_error, err.what()));
            }
            return rp;
        },

        [=](conform_atom,
            const std::string &conform_task,
            const utility::JsonStore &conform_operations,
            const UuidActor &playlist,
            const UuidActor &container,
            const std::string &item_type,
            const UuidActorVector &items,
            const UuidVector &insert_before) -> result<ConformReply> {
            // make worker gather all the information
            auto rp      = make_response_promise<ConformReply>();
            auto ritems  = std::vector<ConformRequestItem>();
            size_t count = 0;

            if (items.empty()) {
                rp.deliver(ConformReply());
            } else {
                for (const auto &i : items) {
                    auto before = Uuid();
                    if (insert_before.size() > count) {
                        before = insert_before.at(count);
                    }
                    count++;

                    if (item_type == "Clip") {
                        ritems.emplace_back(std::make_tuple(i, UuidActor(), before));
                    } else if (item_type == "Media") {
                        ritems.emplace_back(std::make_tuple(i, i, before));
                    } else {
                        rp.deliver(make_error(sec::runtime_error, "Unknown item type"));
                    }
                }

                if (rp.pending()) {
                    auto crequest = ConformRequest(playlist, container, item_type, ritems);
                    crequest.operations_ = conform_operations;
                    conform_step_get_playlist_json(rp, conform_task, crequest);
                }
            }
            return rp;
        },

        [=](conform_atom, const ConformRequest &request) -> result<ConformReply> {
            auto rp = make_response_promise<ConformReply>();
            try {
                if (not conformers.empty()) {
                    // request.dump();
                    fan_out_request<policy::select_all>(
                        conformers, infinite, conform_atom_v, request)
                        .then(
                            [=](const std::vector<ConformReply> all_results) mutable {
                                // compile results..
                                auto result = ConformReply(request);
                                result.items_.resize(request.items_.size());

                                for (const auto &i : all_results) {
                                    if (not i.items_.empty()) {
                                        // map actions already taken
                                        result.operations_.update(i.operations_);

                                        // spdlog::warn("{} {}", request.items_.size(),
                                        // i.items_.size()); insert values into result.
                                        auto count = 0;
                                        for (const auto &j : i.items_) {
                                            // replace, don't sum results, so we only expect one
                                            // result set in total from a plugin.
                                            if (j and not result.items_[count]) {
                                                // spdlog::warn("add result item{}", count);
                                                result.items_[count] = j;
                                            }
                                            count++;
                                        }
                                    }
                                }

                                result.request_ = request;

                                // this is where the magic happens..
                                // we've got a list of media
                                // and a list of clips associated with them.

                                // we need to construct N tracks based off the supplied track
                                // replacing clip media with our media. we also need to rewite
                                // the result items with the new clip actors.
                                auto tracks = std::list<timeline::Item>();

                                for (size_t i = 0; i < request.items_.size(); i++) {
                                    auto media = std::get<0>(request.items_.at(i));

                                    // spdlog::warn("REQUESTED MEDIA {} {}",
                                    // to_string(media.uuid()),
                                    // static_cast<bool>(result.items_.at(i)));


                                    if (result.items_.at(i)) {
                                        // clip matched.

                                        // make sure media is in timeline.
                                        anon_send(
                                            result.request_.container_.actor(),
                                            playlist::add_media_atom_v,
                                            media,
                                            Uuid());

                                        // spdlog::warn("ADD MEDIA to SEQEUNCE {}",
                                        // to_string(media.uuid()));

                                        auto replacemode = result.request_.operations_.value(
                                            "replace_clip", false);
                                        // replace clip..
                                        for (const auto &c : *(result.items_[i])) {
                                            const auto clip_uuid = std::get<0>(c).uuid();
                                            // spdlog::warn("{}", to_string(clip_uuid));
                                            // for each clip asside them to this media.
                                            auto trackit = tracks.begin();

                                            while (true) {
                                                // still iterating
                                                if (trackit == tracks.end()) {
                                                    // are we still bound to the real timeline ?
                                                    auto tmp = result.request_.track_;
                                                    tmp.reset_actor(true);

                                                    // zero out clip media.
                                                    auto clip_items =
                                                        tmp.find_all_items(timeline::IT_CLIP);
                                                    for (auto &i : clip_items) {
                                                        auto prop          = i.get().prop();
                                                        prop["media_uuid"] = Uuid();
                                                        i.get().set_prop(prop);
                                                    }
                                                    tracks.push_back(tmp);
                                                    trackit = tracks.begin();
                                                    std::next(trackit, tracks.size() - 1);
                                                }

                                                auto clipit =
                                                    find_uuid(trackit->children(), clip_uuid);
                                                auto clip_prop = clipit->prop();
                                                if (clip_prop.value("media_uuid", Uuid())
                                                        .is_null()) {
                                                    clip_prop["media_uuid"] = media.uuid();
                                                    clipit->set_prop(clip_prop);

                                                    if (replacemode and
                                                        trackit == tracks.begin()) {
                                                        // find in source and really change it..
                                                        // find clip actor..
                                                        auto real_clip = find_item(
                                                            result.request_.track_.children(),
                                                            clip_uuid);
                                                        if (real_clip) {
                                                            // spdlog::warn("{} {}",
                                                            // (*real_clip)->name(),
                                                            // to_string((*real_clip)->actor()));
                                                            anon_send(
                                                                (*real_clip)->actor(),
                                                                timeline::link_media_atom_v,
                                                                media);
                                                        }
                                                    }

                                                    break;
                                                } else
                                                    trackit++;
                                            }
                                        }
                                    }
                                }

                                // create new track actors and append to timeline.
                                scoped_actor sys{system()};

                                try {
                                    auto track_actors = UuidActorVector();
                                    auto mediamap     = UuidActorMap();

                                    for (const auto &i : request.items_)
                                        mediamap[std::get<0>(i).uuid()] =
                                            std::get<0>(i).actor();

                                    if (result.request_.operations_.at("replace_clip") ==
                                            true and
                                        not tracks.empty()) {
                                        // pop first track.. as this is a duplicate of our
                                        // replacement track. force relink..
                                        // request_receive<bool>(
                                        //     *sys,
                                        //     result.request_.track_.actor(),
                                        //     timeline::link_media_atom_v,
                                        //     mediamap,
                                        //     true);

                                        tracks.pop_front();
                                    }

                                    auto new_track_name = result.request_.operations_.value(
                                        "new_track_name", std::string());
                                    for (auto &i : tracks) {
                                        i.clean(true);
                                        i.reset_uuid(true);
                                        if (not new_track_name.empty())
                                            i.set_name(new_track_name);
                                        // reset id's..
                                        auto track_actor = spawn<timeline::TrackActor>(i, i);
                                        // spdlog::warn("NEW TRACK {} {} {}",
                                        // to_string(track_actor), to_string(i.actor()),
                                        // to_string(i.uuid()));
                                        track_actors.push_back(i.uuid_actor());

                                        request_receive<bool>(
                                            *sys,
                                            track_actor,
                                            timeline::link_media_atom_v,
                                            mediamap,
                                            true);
                                    }

                                    if (not track_actors.empty()) {
                                        // get stack actor
                                        auto stack = request_receive<timeline::Item>(
                                            *sys,
                                            result.request_.container_.actor(),
                                            timeline::item_atom_v,
                                            0);

                                        std::reverse(track_actors.begin(), track_actors.end());
                                        request_receive<JsonStore>(
                                            *sys,
                                            stack.actor(),
                                            timeline::insert_item_atom_v,
                                            0,
                                            track_actors);
                                    }
                                    // rebind media..
                                    // does the timeline even know at this point ?
                                    // anon_send(result.request_.container_.actor(),
                                    // timeline::link_media_atom_v, true);

                                } catch (...) {
                                }

                                rp.deliver(result);
                            },
                            [=](const error &err) mutable {
                                spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                                rp.deliver(err);
                            });

                } else {
                    rp.deliver(ConformReply(request));
                }
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                rp.deliver(make_error(sec::runtime_error, err.what()));
            }

            return rp;
        },

        [=](conform_atom,
            const std::string &conform_task,
            const ConformRequest &request) -> result<ConformReply> {
            if (not conformers.empty()) {
                // request.dump();

                auto rp = make_response_promise<ConformReply>();
                fan_out_request<policy::select_all>(
                    conformers, infinite, conform_atom_v, conform_task, request)
                    .then(
                        [=](const std::vector<ConformReply> all_results) mutable {
                            // compile results..
                            auto result = ConformReply(request);
                            result.items_.resize(request.items_.size());

                            for (const auto &i : all_results) {
                                if (not i.items_.empty()) {
                                    // map actions already taken
                                    result.operations_.update(i.operations_);

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

                            result.request_ = request;

                            // check for deletion request.
                            if (result.request_.item_type_ != "Clip" and
                                result.operations_.at("remove_media") == false and
                                result.request_.operations_.at("remove_media") == true) {
                                // not removed and is requested to remove..
                                // we only remove if there is a replacement found
                                // iterate over source media, remove if results found.
                                result.operations_["remove_media"] = true;
                                auto count                         = 0;
                                for (const auto &i : result.request_.items_) {
                                    if (result.items_[count] and
                                        not result.items_[count]->empty()) {
                                        anon_send(
                                            result.request_.container_.actor(),
                                            playlist::remove_media_atom_v,
                                            std::get<1>(i).uuid());
                                    }
                                    count++;
                                }
                            }

                            if (result.request_.item_type_ == "Clip" and
                                result.operations_.at("replace_clip") == false and
                                result.request_.operations_.at("replace_clip") == true) {
                                // not removed and is requested to remove..
                                // we only remove if there is a replacement found
                                // iterate over source media, remove if results found.
                                result.operations_["replace_clip"] = true;
                                auto count                         = 0;
                                for (const auto &i : result.request_.items_) {
                                    if (result.items_.at(count) and
                                        not result.items_.at(count)->empty()) {
                                        const auto &ritems = *(result.items_.at(count));
                                        const UuidActor &m = std::get<0>(ritems.at(0));

                                        anon_send(
                                            std::get<0>(i).actor(),
                                            timeline::link_media_atom_v,
                                            m);
                                    }
                                    count++;
                                }
                            }

                            if (result.request_.item_type_ == "Clip" and
                                result.operations_.at("replace_clip") == true and
                                result.request_.operations_.at("remove_failed_clip") == true) {

                                // remove clips that had no results.
                                result.operations_["remove_failed_clip"] = true;
                                auto count                               = 0;
                                for (const auto &i : result.request_.items_) {
                                    if (not result.items_[count] or
                                        result.items_[count]->empty()) {
                                        // candidate..
                                        // send deletion request to timeline using clips
                                        // id/actor uuid spdlog::warn("send delete to {} {} for
                                        // {} {}",
                                        //     to_string(result.request_.container_.actor()),
                                        //     to_string(result.request_.container_.uuid()),
                                        //     to_string(std::get<0>(i).actor()),
                                        //     to_string(std::get<0>(i).uuid())
                                        // );
                                        anon_send(
                                            result.request_.container_.actor(),
                                            timeline::erase_item_atom_v,
                                            std::get<0>(i).uuid(),
                                            true,
                                            true);
                                    }
                                    count++;
                                }
                            }

                            // we've got a list of media actors ?
                            // if we're dealing with clips we should replace them ?

                            rp.deliver(result);
                        },
                        [=](const error &err) mutable { rp.deliver(err); });
                return rp;
            }

            return ConformReply(request);
        });
}


// if (remove and not result.empty()) {
//     request_receive<bool>(*sys, cactor, playlist::remove_media_atom_v, iuuid);
// }

void ConformWorkerActor::conform_chain(
    caf::typed_response_promise<ConformReply> rp, ConformRequest &conform_request) {
    try {
        // we step through various actions until we're done.
        auto clip_items = conform_request.track_.find_all_items(timeline::IT_CLIP);
        bool require_clip_media_metadata = false;
        bool require_media_metadata      = false;

        for (const auto &i : clip_items) {
            auto clip_media_uuid = i.get().prop().value("media_uuid", Uuid());
            if (not clip_media_uuid.is_null() and
                not conform_request.metadata_.count(clip_media_uuid)) {
                require_clip_media_metadata = true;
                break;
            }
        }

        for (const auto &i : conform_request.items_) {
            if (not conform_request.metadata_.count(std::get<0>(i).uuid())) {
                require_media_metadata = true;
                break;
            }
        }

        // populate playlist
        if (not conform_request.metadata_.count(conform_request.playlist_.uuid())) {
            request(
                conform_request.playlist_.actor(), infinite, json_store::get_json_atom_v, "")
                .then(
                    [=](const JsonStore &playlist_metadata) mutable {
                        // we maybe processing timeline items..
                        conform_request.metadata_[conform_request.playlist_.uuid()] =
                            playlist_metadata;
                        conform_chain(rp, conform_request);
                    },
                    [=](const error &err) mutable {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                        rp.deliver(err);
                    });
            // get track clip media metadata
        } else if (
            not clip_items.empty() and
            not conform_request.metadata_.count(clip_items.begin()->get().uuid())) {
            // request clip media from track
            for (const auto &i : clip_items)
                conform_request.metadata_[i.get().uuid()] = i.get().prop();
            conform_chain(rp, conform_request);
        } else if (require_clip_media_metadata) {
            // request clip media from track
            // add clip actors with unique media
            auto cactors     = std::vector<caf::actor>();
            auto media_uuids = std::set<utility::Uuid>();
            for (const auto &i : clip_items) {
                auto uuid = i.get().prop().value("media_uuid", Uuid());
                if (not uuid.is_null() and not media_uuids.count(uuid)) {
                    media_uuids.insert(uuid);
                    cactors.push_back(i.get().actor());
                }
            }

            if (not cactors.empty()) {
                // spdlog::warn("get media from clips {}", cactors.size());
                // get associated media actor for clip..
                fan_out_request<policy::select_all>(
                    cactors, infinite, playlist::get_media_atom_v, true)
                    .then(
                        [=](const std::vector<UuidUuidActor> item_media) mutable {
                            // got list of clip media actors.
                            // in this mode we just want the metadata..
                            auto mactors = std::vector<caf::actor>();
                            for (const auto &i : item_media) {
                                mactors.push_back(i.second.actor());
                            }

                            if (not mactors.empty()) {
                                // spdlog::warn("get media from clips {}", mactors.size());

                                fan_out_request<policy::select_all>(
                                    mactors,
                                    infinite,
                                    json_store::get_json_atom_v,
                                    utility::Uuid(),
                                    "",
                                    true)
                                    .then(
                                        [=](const std::vector<std::pair<UuidActor, JsonStore>>
                                                media_metadata) mutable {
                                            // add media metadata to request.

                                            // put json into map key media uuid
                                            for (const auto &i : media_metadata) {
                                                // spdlog::warn("{} {}",
                                                // to_string(i.first.uuid()), i.second.dump(2));
                                                conform_request.metadata_[i.first.uuid()] =
                                                    i.second;
                                            }

                                            conform_chain(rp, conform_request);
                                        },
                                        [=](const error &err) mutable {
                                            spdlog::warn(
                                                "{} {}", __PRETTY_FUNCTION__, to_string(err));
                                            rp.deliver(err);
                                        });

                            } else {
                                rp.deliver(
                                    make_error(sec::runtime_error, "Empty media json help."));
                            }
                        },
                        [=](const error &err) mutable {
                            spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                            rp.deliver(err);
                        });

            } else {
                rp.deliver(make_error(sec::runtime_error, "No clip actors ?"));
            }

        } else if (require_media_metadata) {
            // if item != media it's a clip/media pair.
            // in this scope it's always media..
            auto mactors = std::vector<caf::actor>();
            for (const auto &i : conform_request.items_) {
                mactors.push_back(std::get<0>(i).actor());
            }

            if (not mactors.empty()) {
                fan_out_request<policy::select_all>(
                    mactors, infinite, json_store::get_json_atom_v, utility::Uuid(), "", true)
                    .then(
                        [=](const std::vector<std::pair<UuidActor, JsonStore>>
                                media_metadata) mutable {
                            // add media metadata to request.

                            // put json into map key media uuid
                            for (const auto &i : media_metadata)
                                conform_request.metadata_[i.first.uuid()] = i.second;

                            conform_chain(rp, conform_request);
                        },
                        [=](const error &err) mutable {
                            spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                            rp.deliver(err);
                        });

            } else {
                rp.deliver(make_error(sec::runtime_error, "Empty media json help."));
            }
        } else {
            rp.delegate(caf::actor_cast<caf::actor>(this), conform_atom_v, conform_request);
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        rp.deliver(make_error(sec::runtime_error, err.what()));
    }
}

void ConformWorkerActor::conform_step_get_playlist_json(
    caf::typed_response_promise<ConformReply> rp,
    const std::string &conform_task,
    ConformRequest conform_request) {
    // spdlog::warn("conform_step_get_playlist_json");
    // conform_request.dump();

    request(conform_request.playlist_.actor(), infinite, json_store::get_json_atom_v, "")
        .then(
            [=](const JsonStore &playlist_metadata) mutable {
                // we maybe processing timeline items..
                // conform_request.metadata_[conform_request.playlist_.uuid()] =
                // playlist_metadata;

                if (conform_request.item_type_ == "Clip") {
                    // neet to get clip meta data and clip media if they exist..
                    conform_step_get_clip_json(rp, conform_task, conform_request);
                } else if (conform_request.item_type_ == "Media") {
                    conform_step_get_media_json(rp, conform_task, conform_request);
                } else {
                    rp.deliver(make_error(sec::runtime_error, "Unknown item type"));
                }
            },
            [=](const error &err) mutable {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                rp.deliver(err);
            });
}

void ConformWorkerActor::conform_step_get_clip_json(
    caf::typed_response_promise<ConformReply> rp,
    const std::string &conform_task,
    ConformRequest &conform_request) {
    // spdlog::warn("conform_step_get_clip_json");
    // conform_request.dump();

    // request clip meta data.
    auto actors = std::vector<caf::actor>();
    for (const auto &i : conform_request.items_)
        actors.push_back(std::get<0>(i).actor());

    fan_out_request<policy::select_all>(actors, infinite, timeline::item_atom_v)
        .then(
            [=](const std::vector<timeline::Item> items) mutable {
                for (const auto &i : items)
                    conform_request.metadata_[i.uuid()] = i.prop();

                conform_step_get_clip_media(rp, conform_task, conform_request);
            },
            [=](const error &err) mutable {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                rp.deliver(err);
            });
}

void ConformWorkerActor::conform_step_get_clip_media(
    caf::typed_response_promise<ConformReply> rp,
    const std::string &conform_task,
    ConformRequest &conform_request) {
    // spdlog::warn("conform_step_get_clip_media");
    // conform_request.dump();

    // request clip meta data.
    auto actors = std::vector<caf::actor>();
    for (const auto &i : conform_request.items_)
        actors.push_back(std::get<0>(i).actor());

    // get associated media actor for clip..
    fan_out_request<policy::select_all>(actors, infinite, playlist::get_media_atom_v, true)
        .then(
            [=](const std::vector<UuidUuidActor> item_media) mutable {
                auto clip_media_map = std::map<utility::Uuid, UuidActor>();

                for (const auto &i : item_media) {
                    if (i.second.actor())
                        clip_media_map[i.first] = i.second;
                }

                for (auto &i : conform_request.items_) {
                    auto cmedia = clip_media_map.find(std::get<0>(i).uuid());
                    if (cmedia != std::end(clip_media_map))
                        std::get<1>(i) = cmedia->second;
                }

                conform_step_get_media_json(rp, conform_task, conform_request);
            },
            [=](const error &err) mutable {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                rp.deliver(err);
            });
}


void ConformWorkerActor::conform_step_get_media_json(
    caf::typed_response_promise<ConformReply> rp,
    const std::string &conform_task,
    ConformRequest &conform_request) {

    // spdlog::warn("conform_step_get_media_json");
    // conform_request.dump();

    auto actors = std::vector<caf::actor>();
    for (const auto &i : conform_request.items_) {
        auto actor = std::get<1>(i).actor();
        if (actor)
            actors.push_back(std::get<1>(i).actor());
    }

    if (not actors.empty()) {
        fan_out_request<policy::select_all>(
            actors, infinite, json_store::get_json_atom_v, utility::Uuid(), "", true)
            .then(
                [=](const std::vector<std::pair<UuidActor, JsonStore>> media_metadata) mutable {
                    // add media metadata to request.

                    // put json into map key media uuid
                    for (const auto &i : media_metadata)
                        conform_request.metadata_[i.first.uuid()] = i.second;

                    // also get source names..
                    conform_step_get_media_source(rp, conform_task, conform_request);
                },
                [=](const error &err) mutable {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                    rp.deliver(err);
                });
    } else {
        // no media..
        rp.delegate(
            caf::actor_cast<caf::actor>(this), conform_atom_v, conform_task, conform_request);
    }
}

void ConformWorkerActor::conform_step_get_media_source(
    caf::typed_response_promise<ConformReply> rp,
    const std::string &conform_task,
    ConformRequest &conform_request) {
    // spdlog::warn("conform_step_get_media_source");

    // get media actors
    auto actors = std::vector<caf::actor>();
    for (const auto &i : conform_request.items_)
        actors.push_back(std::get<1>(i).actor());

    fan_out_request<policy::select_all>(
        actors, infinite, media::current_media_source_atom_v, true)
        .then(
            [=](const std::vector<std::pair<UuidActor, std::pair<std::string, std::string>>>
                    media_sources) mutable {
                for (const auto &i : media_sources) {
                    if (conform_request.metadata_.count(i.first.uuid())) {
                        conform_request.metadata_[i.first.uuid()]["metadata"]["image_source"] =
                            i.second.first;
                        conform_request.metadata_[i.first.uuid()]["metadata"]["audio_source"] =
                            i.second.second;
                    }
                }


                // std::map<Uuid, std::pair<std::string, std::string>> source_map;

                // for (const auto &i : media_sources)
                //     source_map[i.first.uuid()] = i.second;

                // for (auto &i : conform_request.items_) {
                //     auto msource = source_map.find(std::get<1>(i).uuid());
                //     if (msource != std::end(source_map)) {
                //         std::get<1>(i)["metadata"]["image_source"] = msource->second.first;
                //         std::get<1>(i)["metadata"]["audio_source"] = msource->second.second;
                //     }
                // }

                rp.delegate(
                    caf::actor_cast<caf::actor>(this),
                    conform_atom_v,
                    conform_task,
                    conform_request);
            },
            [=](const error &err) mutable {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                rp.deliver(err);
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


        [=](conform_atom, const std::string &conform_task, const ConformRequest &request) {
            delegate(pool_, conform_atom_v, conform_task, request);
        },

        [=](conform_atom, const UuidActor &timeline) {
            delegate(pool_, conform_atom_v, timeline);
        },

        [=](conform_atom,
            const std::string &conform_task,
            const utility::JsonStore &conform_operations,
            const UuidActor &playlist,
            const UuidActor &container,
            const std::string &item_type,
            const UuidActorVector &items,
            const UuidVector &insert_before) {
            delegate(
                pool_,
                conform_atom_v,
                conform_task,
                conform_operations,
                playlist,
                container,
                item_type,
                items,
                insert_before);
        },

        [=](conform_atom,
            const utility::JsonStore &conform_operations,
            const UuidActor &playlist,
            const UuidActor &timeline,
            const UuidActor &conform_track,
            const UuidActorVector &media) {
            delegate(
                pool_,
                conform_atom_v,
                conform_operations,
                playlist,
                timeline,
                conform_track,
                media);
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
