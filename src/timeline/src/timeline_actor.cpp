// SPDX-License-Identifier: Apache-2.0
#include <caf/policy/select_all.hpp>

#ifdef BUILD_OTIO
#include <opentimelineio/version.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/clip.h>
#include <opentimelineio/marker.h>
#include <opentimelineio/track.h>
#include <opentimelineio/externalReference.h>
#endif

#include <cpp-colors/colors.h>

#include "xstudio/atoms.hpp"
#include "xstudio/bookmark/bookmark_actor.hpp"
#include "xstudio/broadcast/broadcast_actor.hpp"
#include "xstudio/history/history_actor.hpp"
#include "xstudio/media/media_actor.hpp"
#include "xstudio/playhead/playhead_actor.hpp"
#include "xstudio/playhead/playhead_selection_actor.hpp"
#include "xstudio/timeline/clip_actor.hpp"
#include "xstudio/timeline/stack_actor.hpp"
#include "xstudio/timeline/gap_actor.hpp"
#include "xstudio/timeline/timeline_actor.hpp"
#include "xstudio/timeline/track_actor.hpp"
#include "xstudio/utility/chrono.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/uuid.hpp"

using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::timeline;
using namespace caf;

namespace {

auto __sysclock_now() {
#ifdef _MSC_VER
    auto tp = sysclock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
#else
    return sysclock::now();
#endif
}


} // namespace

caf::actor
TimelineActor::deserialise(const utility::JsonStore &value, const bool replace_item) {
    auto key   = utility::Uuid(value.at("base").at("item").at("uuid"));
    auto actor = caf::actor();

    if (value.at("base").at("container").at("type") == "Stack") {
        auto item = Item();
        actor     = spawn<StackActor>(static_cast<utility::JsonStore>(value), item);
        add_item(UuidActor(key, actor));
        if (replace_item) {
            auto itemit = find_uuid(base_.item().children(), key);

            if (itemit != base_.item().end()) {
                (*itemit) = item;
            } else {
                spdlog::warn(
                    "{} Invalid item to replace {} {}",
                    __PRETTY_FUNCTION__,
                    to_string(key),
                    value.dump(2));
            }
        }
    }

    return actor;
}

// trigger actor creation
void TimelineActor::item_post_event_callback(const utility::JsonStore &event, Item &item) {

    switch (static_cast<ItemAction>(event.at("action"))) {
    case IA_INSERT: {
        auto cuuid = utility::Uuid(event.at("item").at("uuid"));
        // spdlog::warn("{} {} {} {}", find_uuid(base_.item().children(), cuuid) !=
        // base_.item().cend(), actors_.count(cuuid), not event["blind"].is_null(),
        // event.dump(2)); needs to be child..
        auto child_item_it = find_uuid(base_.item().children(), cuuid);
        if (child_item_it != base_.item().cend() and not actors_.count(cuuid) and
            not event.at("blind").is_null()) {
            // our child
            // spdlog::warn("RECREATE MATCH");

            auto actor = deserialise(utility::JsonStore(event.at("blind")), false);
            add_item(UuidActor(cuuid, actor));
            // spdlog::warn("{}",to_string(caf::actor_cast<caf::actor_addr>(actor)));
            // spdlog::warn("{}",to_string(caf::actor_cast<caf::actor_addr>(child_item_it->actor())));
            child_item_it->set_actor_addr(actor);
            // change item actor addr
            // spdlog::warn("{}",to_string(caf::actor_cast<caf::actor_addr>(child_item_it->actor())));

            // item actor_addr will be wrong.. in ancestors
            // send special update..
            send(
                event_group_,
                event_atom_v,
                item_atom_v,
                child_item_it->make_actor_addr_update(),
                true);
        }
        // spdlog::warn("TimelineActor IT_INSERT");
        // rebuilt child.. trigger relink
    } break;

    case IA_REMOVE: {
        auto cuuid = utility::Uuid(event.at("item_uuid"));
        // child destroyed
        if (actors_.count(cuuid)) {
            // spdlog::warn("destroy
            // {}",to_string(caf::actor_cast<caf::actor_addr>(actors_[cuuid])));
            demonitor(actors_[cuuid]);
            send_exit(actors_[cuuid], caf::exit_reason::user_shutdown);
            actors_.erase(cuuid);
        }
    } break;

    case IA_PROP:
    case IA_NONE:
    case IA_ENABLE:
    case IA_ADDR:
    case IA_RANGE:
    case IA_ACTIVE:
    case IA_AVAIL:
    case IA_SPLICE:
    case IA_NAME:
    case IA_FLAG:
    case IA_LOCK:
    default:
        break;
    }
}

void TimelineActor::item_pre_event_callback(const utility::JsonStore &event, Item &item) {
    try {

        switch (static_cast<ItemAction>(event.at("action"))) {
        case IA_REMOVE: {
            auto cuuid = utility::Uuid(event.at("item_uuid"));
            // spdlog::warn("{}", event.dump(2));
            // child destroyed
            if (actors_.count(cuuid)) {
            } else {
                // watch for clip deletion events
                // we'll want to check for media cleanup required.
                auto citem = item.item_at_index(event.value("index", 0));
                if (citem and (*citem)->item_type() == IT_CLIP) {
                    auto media_uuid = (*citem)->prop().value("media_uuid", utility::Uuid());
                    // get a count of all references to this media..
                    // more than one then nothing to do.
                    auto media_clips = find_media_clips(base_.children(), media_uuid);
                    if (media_clips.size() == 1)
                        delayed_anon_send(
                            actor_cast<caf::actor>(this),
                            std::chrono::milliseconds(100),
                            playlist::remove_media_atom_v,
                            utility::UuidVector({media_uuid}));
                } else {
                    // we need to find all clip items under this item that's being removed.
                    // collect a map of all media_uuids about to be removed.
                    auto mmap  = std::map<utility::Uuid, size_t>();
                    auto clips = (*citem)->find_all_items(IT_CLIP);
                    for (const auto &i : clips) {
                        auto media_uuid = i.get().prop().value("media_uuid", utility::Uuid());
                        if (not media_uuid.is_null()) {
                            if (not mmap.count(media_uuid))
                                mmap[media_uuid] = 0;
                            mmap[media_uuid]++;
                        }
                    }

                    for (const auto &i : mmap) {
                        auto media_clips = find_media_clips(base_.children(), i.first);
                        if (media_clips.size() == i.second) {
                            delayed_anon_send(
                                actor_cast<caf::actor>(this),
                                std::chrono::milliseconds(500),
                                playlist::remove_media_atom_v,
                                utility::UuidVector({i.first}));
                        }
                    }
                }
            }
        } break;
        case IA_PROP: {
            const auto item_media_uuid = item.prop().value("media_uuid", utility::Uuid());
            const auto new_item_media_uuid =
                event.at("value").value("media_uuid", utility::Uuid());
            if (not item_media_uuid.is_null() and item_media_uuid != new_item_media_uuid) {
                auto media_clips = find_media_clips(base_.children(), item_media_uuid);
                if (media_clips.size() == 1)
                    delayed_anon_send(
                        actor_cast<caf::actor>(this),
                        std::chrono::milliseconds(100),
                        playlist::remove_media_atom_v,
                        utility::UuidVector({item_media_uuid}));
            }
            // spdlog::warn("item_pre_event_callback {} {} {} {}",
            // to_string(item_media_uuid),to_string(new_item_media_uuid),item.name(),
            // event.dump(2));
        } break;


        default:
            break;
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }
}

#ifdef BUILD_OTIO

namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;


// std::vector<Retainer<Composable>> const& children() const noexcept
// {
//     return _children;
// }

// {
//     "OTIO_SCHEMA": "Marker.2",
//     "metadata": {
//         "fcp_xml": {
//             "comment": "1001,1017-1142,1158"
//         }
//     },
//     "name": "064_BMC_0020",
//     "color": "RED",
//     "marked_range": {
//         "OTIO_SCHEMA": "TimeRange.1",
//         "duration": {
//             "OTIO_SCHEMA": "RationalTime.1",
//             "rate": 24.0,
//             "value": 70.0
//         },
//         "start_time": {
//             "OTIO_SCHEMA": "RationalTime.1",
//             "rate": 24.0,
//             "value": 0.0
//         }
//     }
// }
std::vector<timeline::Marker>
process_markers(const std::vector<otio::SerializableObject::Retainer<otio::Marker>> &markers) {
    auto result = std::vector<timeline::Marker>();

    for (const auto &om : markers) {
        auto m = Marker(om->name());

        auto marker_metadata = JsonStore();
        try {
            otio::ErrorStatus err;
            auto marker_metadata = nlohmann::json::parse(om->to_json_string(&err, {}, 0));
            if (marker_metadata.count("metadata"))
                m.set_prop(JsonStore(marker_metadata.at("metadata")));
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }

        if (colors::wpf_named_color_converter::is_named(om->color()))
            m.set_flag(colors::to_ahex_str<char>(colors::color(
                colors::value_of(colors::wpf_named_color_converter::value(om->color())))));

        auto marked_range = om->marked_range();
        m.set_range(FrameRange(
            FrameRateDuration(
                static_cast<int>(marked_range.start_time().value()),
                marked_range.start_time().rate()),
            FrameRateDuration(
                static_cast<int>(marked_range.duration().value()),
                marked_range.duration().rate())));

        result.emplace_back(m);
    }

    return result;
}

void process_item(
    const std::vector<otio::SerializableObject::Retainer<otio::Composable>> &items,
    blocking_actor *self,
    caf::actor &parent,
    const std::map<std::string, UuidActor> &media_lookup) {

    auto fcp_locked_path     = nlohmann::json::json_pointer("/metadata/fcp_xml/locked");
    auto fcp_enabled_path    = nlohmann::json::json_pointer("/metadata/fcp_xml/enabled");
    auto fcp_track_name_path = nlohmann::json::json_pointer("/metadata/fcp_xml/@MZ.TrackName");

    // let the fun begin..
    for (auto i : items) {
        if (auto ii = dynamic_cast<otio::Track *>(&(*i))) {
            // spdlog::warn("Track");
            auto media_type = media::MediaType::MT_IMAGE;
            if (ii->kind() == otio::Track::Kind::audio)
                media_type = media::MediaType::MT_AUDIO;

            auto locked   = false;
            auto enabled  = true;
            auto name     = ii->name();
            auto metadata = JsonStore();
            try {
                // "fcp_xml": {
                //     "@MZ.TrackName": "Cut Ref QT",
                //     "@MZ.TrackTargeted": "1",
                //     "@TL.SQTrackExpanded": "0",
                //     "@TL.SQTrackExpandedHeight": "45",
                //     "@TL.SQTrackShy": "0",
                //     "enabled": "TRUE",
                //     "locked": "FALSE"
                //   }
                otio::ErrorStatus err;
                auto track_metadata = nlohmann::json::parse(ii->to_json_string(&err, {}, 0));

                // if(track_metadata.count("metadata")) {
                //     spdlog::warn("{}", track_metadata.at("metadata").dump(2));
                // }

                if (track_metadata.contains(fcp_locked_path) and
                    track_metadata.at(fcp_locked_path).get<std::string>() == "TRUE")
                    locked = true;

                if (track_metadata.contains(fcp_enabled_path) and
                    track_metadata.at(fcp_enabled_path).get<std::string>() == "FALSE")
                    enabled = false;

                if (track_metadata.contains(fcp_track_name_path) and
                    track_metadata.at(fcp_track_name_path).get<std::string>() != "")
                    name = track_metadata.at(fcp_track_name_path).get<std::string>();

                if (track_metadata.count("metadata"))
                    metadata = JsonStore(track_metadata.at("metadata"));

            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }

            auto uuid  = Uuid::generate();
            auto actor = self->spawn<TrackActor>(name, media_type, uuid);

            if (locked)
                self->request(actor, infinite, item_lock_atom_v, locked)
                    .receive([=](const JsonStore &) {}, [=](const error &err) {});

            if (not enabled)
                self->request(actor, infinite, plugin_manager::enable_atom_v, enabled)
                    .receive([=](const JsonStore &) {}, [=](const error &err) {});

            anon_send(actor, item_prop_atom_v, metadata, "");

            auto source_range = ii->source_range();
            if (source_range)
                self->request(
                        actor,
                        infinite,
                        active_range_atom_v,
                        FrameRange(FrameRateDuration(
                            static_cast<int>(source_range->duration().value()),
                            source_range->duration().rate())))
                    .receive([=](const JsonStore &) {}, [=](const error &err) {});

            self->request(
                    parent,
                    infinite,
                    insert_item_atom_v,
                    -1,
                    UuidActorVector({UuidActor(uuid, actor)}))
                .receive([=](const JsonStore &) {}, [=](const error &err) {});

            process_item(ii->children(), self, actor, media_lookup);
            // } else if (ii->kind() == otio::Track::Kind::audio) {
            //     // spdlog::warn("Audio Track");
            //     auto uuid = Uuid::generate();
            //     auto actor =
            //         self->spawn<TrackActor>(ii->name(), media::MediaType::MT_AUDIO, uuid);
            //     auto source_range = ii->source_range();

            //     if (source_range)
            //         self->request(
            //                 actor,
            //                 infinite,
            //                 active_range_atom_v,
            //                 FrameRange(FrameRateDuration(
            //                     static_cast<int>(source_range->duration().value()),
            //                     source_range->duration().rate())))
            //             .receive([=](const JsonStore &) {}, [=](const error &err) {});

            //     self->request(
            //             parent,
            //             infinite,
            //             insert_item_atom_v,
            //             -1,
            //             UuidActorVector({UuidActor(uuid, actor)}))
            //         .receive([=](const JsonStore &) {}, [=](const error &err) {});

            //     process_item(ii->children(), self, actor, media_lookup);
            // }
        } else if (auto ii = dynamic_cast<otio::Gap *>(&(*i))) {

            auto uuid  = Uuid::generate();
            auto actor = self->spawn<GapActor>(ii->name(), utility::FrameRateDuration(), uuid);
            auto source_range = ii->source_range();

            if (source_range)
                self->request(
                        actor,
                        infinite,
                        active_range_atom_v,
                        FrameRange(FrameRateDuration(
                            static_cast<int>(source_range->duration().value()),
                            source_range->duration().rate())))
                    .receive([=](const JsonStore &) {}, [=](const error &err) {});

            self->request(
                    parent,
                    infinite,
                    insert_item_atom_v,
                    -1,
                    UuidActorVector({UuidActor(uuid, actor)}))
                .receive([=](const JsonStore &) {}, [=](const error &err) {});

        } else if (auto ii = dynamic_cast<otio::Clip *>(&(*i))) {
            // spdlog::warn("Clip");
            // what does it contain ?
            auto uuid    = Uuid::generate();
            auto actor   = caf::actor();
            auto enabled = true;

            const auto active_key = ii->active_media_reference_key();
            auto active_path      = std::string();

            if (auto active = otio::SerializableObject::Retainer<otio::ExternalReference>(
                    dynamic_cast<otio::ExternalReference *>(ii->media_reference()))) {
                active_path = active->target_url();
            }

            if (active_path.empty() or not media_lookup.count(active_path)) {
                // spdlog::warn("ERRRR {}", active_path);
                // missing media..
                actor = self->spawn<ClipActor>(UuidActor(), ii->name(), uuid);
            } else {
                actor = self->spawn<ClipActor>(media_lookup.at(active_path), ii->name(), uuid);

                // should come from media...

                // self->request(
                //         actor,
                //         infinite,
                //         available_range_atom_v,
                //         FrameRange(
                //             FrameRateDuration(
                //                 static_cast<int>(ii->available_range().start_time().value()),
                //                 ii->available_range().start_time().rate()),
                //             FrameRateDuration(
                //                 static_cast<int>(ii->available_range().duration().value()),
                //                 ii->available_range().duration().rate())))
                //     .receive([=](const JsonStore &) {}, [=](const error &err) {});
            }

            auto metadata = JsonStore();
            try {
                otio::ErrorStatus err;
                auto metadata = nlohmann::json::parse(ii->to_json_string(&err, {}, 0));

                if (metadata.contains(fcp_enabled_path) and
                    metadata.at(fcp_enabled_path).get<std::string>() == "FALSE")
                    enabled = false;

                if (metadata.count("metadata"))
                    anon_send(actor, item_prop_atom_v, JsonStore(metadata.at("metadata")), "");
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }

            if (not enabled)
                self->request(actor, infinite, plugin_manager::enable_atom_v, enabled)
                    .receive([=](const JsonStore &) {}, [=](const error &err) {});

            auto source_range = ii->source_range();
            if (source_range) {
                self->request(
                        actor,
                        infinite,
                        active_range_atom_v,
                        FrameRange(
                            FrameRateDuration(
                                static_cast<int>(source_range->start_time().value()),
                                source_range->start_time().rate()),
                            FrameRateDuration(
                                static_cast<int>(source_range->duration().value()),
                                source_range->duration().rate())))
                    .receive([=](const JsonStore &) {}, [=](const error &err) {});
            }

            self->request(
                    parent,
                    infinite,
                    insert_item_atom_v,
                    -1,
                    UuidActorVector({UuidActor(uuid, actor)}))
                .receive([=](const JsonStore &) {}, [=](const error &err) {});

        } else if (auto ii = dynamic_cast<otio::Stack *>(&(*i))) {
            // spdlog::warn("Stack");
            // timeline where marker live..
            auto uuid  = Uuid::generate();
            auto actor = self->spawn<StackActor>(ii->name(), uuid);

            auto markers = process_markers(ii->markers());

            if (not markers.empty())
                anon_send(actor, item_marker_atom_v, insert_item_atom_v, markers);


            auto metadata = JsonStore();
            try {
                otio::ErrorStatus err;
                auto metadata = nlohmann::json::parse(ii->to_json_string(&err, {}, 0));
                if (metadata.count("metadata"))
                    anon_send(actor, item_prop_atom_v, JsonStore(metadata.at("metadata")), "");
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }

            auto source_range = ii->source_range();

            if (source_range)
                self->request(
                        actor,
                        infinite,
                        active_range_atom_v,
                        FrameRange(FrameRateDuration(
                            static_cast<int>(source_range->duration().value()),
                            source_range->duration().rate())))
                    .receive([=](const JsonStore &) {}, [=](const error &err) {});

            self->request(
                    parent,
                    infinite,
                    insert_item_atom_v,
                    -1,
                    UuidActorVector({UuidActor(uuid, actor)}))
                .receive([=](const JsonStore &) {}, [=](const error &err) {});

            process_item(ii->children(), self, parent, media_lookup);
        }
    }
}

void timeline_importer(
    blocking_actor *self,
    caf::response_promise rp,
    const caf::actor &playlist,
    const UuidActor &dst,
    const caf::uri &path,
    const std::string &data) {

    otio::ErrorStatus error_status;
    otio::SerializableObject::Retainer<otio::Timeline> timeline;

    timeline = otio::SerializableObject::Retainer<otio::Timeline>(
        dynamic_cast<otio::Timeline *>(otio::Timeline::from_json_string(data, &error_status)));

    if (otio::is_error(error_status)) {
        return rp.deliver(false);
    }

    auto global_start_time = timeline->global_start_time();

    if (global_start_time) {
        self->request(
                dst.actor(),
                infinite,
                available_range_atom_v,
                FrameRange(
                    FrameRateDuration(
                        static_cast<int>(global_start_time->value()),
                        global_start_time->rate()),
                    FrameRateDuration(0, global_start_time->rate())))
            .receive([=](const JsonStore &) {}, [=](const error &err) {});
    }

    auto timeline_metadata = JsonStore(R"({})"_json);
    try {
        otio::ErrorStatus err;
        auto metadata = nlohmann::json::parse(timeline->to_json_string(&err, {}, 0));
        if (metadata.count("metadata"))
            timeline_metadata = metadata.at("metadata");
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }
    timeline_metadata["path"] = to_string(path);

    anon_send(dst.actor(), item_prop_atom_v, timeline_metadata);

    // timeline loaded, convert to native timeline.
    //  iterate over media, and add to playlist.
    const std::vector<otio::SerializableObject::Retainer<otio::Clip>> clips =
        (timeline->find_clips());

    std::map<std::string, UuidActor> target_url_map;

    spdlog::warn("processing {} clips", clips.size());

    for (const auto &cl : clips) {
        const auto &name = cl->name();
        spdlog::warn("{} {}", name, cl->active_media_reference_key());

        const auto active_key = cl->active_media_reference_key();
        auto active_path      = std::string();

        if (auto active = otio::SerializableObject::Retainer<otio::ExternalReference>(
                dynamic_cast<otio::ExternalReference *>(cl->media_reference()))) {
            active_path = active->target_url();
        }

        // spdlog::warn("BLAGH {} {}", active_key, active_path);

        // WARNING this may inadvertantly skip auxiliary sources we want..
        if (active_path.empty() or target_url_map.count(active_path)) {
            // spdlog::warn("SKIP");
            continue;
        }

        auto clip_metadata = JsonStore();
        try {
            otio::ErrorStatus err;
            auto clip_meta = nlohmann::json::parse(cl->to_json_string(&err, {}, 0));
            if (clip_meta.count("metadata")) {
                clip_metadata = JsonStore(clip_meta.at("metadata"));
            }
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }

        // check we're not adding the same media twice.
        UuidActorVector sources;

        // create media sources.
        for (const auto &mr : cl->media_references()) {
            spdlog::warn("BLOB {} {}", mr.first, mr.second->name());
            // try and dynamic cast to
            if (auto ext = otio::SerializableObject::Retainer<otio::ExternalReference>(
                    dynamic_cast<otio::ExternalReference *>(mr.second))) {

                auto uri = caf::make_uri(ext->target_url());
                if (!uri) {
                    uri = posix_path_to_uri(ext->target_url());
                }
                spdlog::warn("FLOB {} {} {}", ext->target_url(), to_string(uri), bool(uri));
                if (uri) {
                    auto extname     = ext->name();
                    auto source_uuid = utility::Uuid::generate();
                    auto rate        = FrameRate();
                    auto ar          = ext->available_range();
                    if (ar) {
                        rate = FrameRate(ar->start_time().rate());
                    }

                    auto source_metadata = JsonStore();
                    try {
                        otio::ErrorStatus err;
                        auto ext_meta = nlohmann::json::parse(ext->to_json_string(&err, {}, 0));
                        if (ext_meta.count("metadata")) {
                            source_metadata = JsonStore(ext_meta.at("metadata"));
                        }
                    } catch (const std::exception &err) {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    }

                    auto source = self->spawn<media::MediaSourceActor>(
                        extname.empty() ? std::string("ExternalReference") : extname,
                        *uri,
                        rate,
                        source_uuid);

                    if (not source_metadata.is_null())
                        anon_send(
                            source,
                            json_store::set_json_atom_v,
                            source_metadata,
                            "/metadata/timeline");

                    sources.emplace_back(UuidActor(source_uuid, source));
                }
            }
        }

        std::cerr << "NUM SOURCES " << sources.size() << " .... " << name << "\n";

        // //  add media.
        if (not sources.empty()) {
            // create media
            // add to map.
            auto uuid = Uuid::generate();
            target_url_map[active_path] =
                UuidActor(uuid, self->spawn<media::MediaActor>(name, uuid, sources));

            if (not clip_metadata.is_null())
                anon_send(
                    target_url_map[active_path].actor(),
                    json_store::set_json_atom_v,
                    clip_metadata,
                    "/metadata/timeline");

            anon_send(
                target_url_map[active_path].actor(),
                media::current_media_source_atom_v,
                sources.front().uuid());
        }

        otio::RationalTime dur = cl->duration();
        std::cout << "Name: " << cl->name() << " [";
        std::cout << dur.value() << "/" << dur.rate() << "]" << std::endl;
        // trigger population of additional sources ? May conflict with timeline ?
    }

    std::cerr << "target_url_map " << target_url_map.size() << "\n";

    // populate source
    if (not target_url_map.empty()) {
        // batch add..
        UuidActorVector new_media;

        for (const auto &i : target_url_map)
            new_media.push_back(i.second);

        // trigger additional sources.
        utility::UuidList media_uuids;
        for (const auto &i : target_url_map) {
            media_uuids.push_back(i.second.uuid());
        }

        // add to playlist/timeline
        self->request(dst.actor(), infinite, playlist::add_media_atom_v, new_media, Uuid())
            .receive([=](const UuidActor &) {}, [=](const error &err) {});
    }

    // process timeline.
    caf::actor history_actor;

    self->request(dst.actor(), infinite, history::history_atom_v)
        .receive(
            [&](const UuidActor &ua) mutable { history_actor = ua.actor(); },
            [=](error &err) { spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err)); });

    // disable history whilst populating
    // should we clear it ?
    if (history_actor) {
        anon_send(history_actor, plugin_manager::enable_atom_v, false);
    }

    // build timeline, for fun and profit..
    // purge any current timeline..
    std::vector<otio::SerializableObject::Retainer<otio::Composable>> tracks;

    auto vtracks = timeline->video_tracks();
    for (auto it = vtracks.rbegin(); it != vtracks.rend(); ++it)
        tracks.emplace_back(otio::SerializableObject::Retainer<otio::Composable>(*it));

    auto atracks = timeline->audio_tracks();
    for (auto &atrack : atracks)
        tracks.emplace_back(otio::SerializableObject::Retainer<otio::Composable>(atrack));

    // get timeline stack..
    auto stack_actor = caf::actor();

    self->request(dst.actor(), infinite, item_atom_v, 0)
        .receive(
            [&](const Item &item) mutable { stack_actor = item.actor(); },
            [=](error &err) { spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err)); });

    if (stack_actor) {
        // process markers on top level stack..
        auto stack = timeline->tracks();

        auto markers = process_markers(stack->markers());

        if (not markers.empty())
            anon_send(stack_actor, item_marker_atom_v, insert_item_atom_v, markers);

        process_item(tracks, self, stack_actor, target_url_map);
    }

    // enable history, we've finished.
    if (history_actor) {
        anon_send(history_actor, plugin_manager::enable_atom_v, true);
    }

    // spdlog::warn("imported");

    rp.deliver(true);
}

#endif // BUILD_OTIO

TimelineActor::TimelineActor(
    caf::actor_config &cfg, const utility::JsonStore &jsn, const caf::actor &playlist)
    : caf::event_based_actor(cfg),
      base_(static_cast<utility::JsonStore>(jsn["base"])),
      playlist_(playlist ? caf::actor_cast<caf::actor_addr>(playlist) : caf::actor_addr()) {
    base_.item().set_actor_addr(this);
    // parse and generate tracks/stacks.

    if (playlist)
        anon_send(this, playhead::source_atom_v, playlist, UuidUuidMap());

    for (const auto &[key, value] : jsn["actors"].items()) {
        try {
            deserialise(value, true);
        } catch (const std::exception &e) {
            spdlog::error("{}", e.what());
        }
    }

    base_.item().set_system(&system());

    base_.item().bind_item_pre_event_func(
        [this](const utility::JsonStore &event, Item &item) {
            item_pre_event_callback(event, item);
        },
        true);
    base_.item().bind_item_post_event_func([this](const utility::JsonStore &event, Item &item) {
        item_post_event_callback(event, item);
    });


    init();
}

TimelineActor::TimelineActor(
    caf::actor_config &cfg,
    const std::string &name,
    const utility::Uuid &uuid,
    const caf::actor &playlist,
    const bool with_tracks)
    : caf::event_based_actor(cfg),
      base_(name, uuid, this),
      playlist_(playlist ? caf::actor_cast<caf::actor_addr>(playlist) : caf::actor_addr()) {

    // create default stack
    auto suuid = Uuid::generate();
    auto stack = spawn<StackActor>("Stack", suuid);
    if (with_tracks) {
        auto vuuid  = Uuid::generate();
        auto auuid  = Uuid::generate();
        auto vactor = spawn<TrackActor>("Video Track", media::MediaType::MT_IMAGE, vuuid);
        auto aactor = spawn<TrackActor>("Audio Track", media::MediaType::MT_AUDIO, auuid);
        anon_send<message_priority::high>(
            stack, insert_item_atom_v, 0, UuidActorVector({UuidActor(vuuid, vactor)}));
        anon_send<message_priority::high>(
            stack, insert_item_atom_v, 1, UuidActorVector({UuidActor(auuid, aactor)}));
    }

    anon_send<message_priority::high>(
        this, insert_item_atom_v, 0, UuidActorVector({UuidActor(suuid, stack)}));
    base_.item().set_system(&system());
    base_.item().set_name(name);

    base_.item().bind_item_pre_event_func(
        [this](const utility::JsonStore &event, Item &item) {
            item_pre_event_callback(event, item);
        },
        true);
    base_.item().bind_item_post_event_func([this](const utility::JsonStore &event, Item &item) {
        item_post_event_callback(event, item);
    });

    init();
}

caf::message_handler TimelineActor::default_event_handler() {
    return {
        [=](utility::event_atom, item_atom, const Item &) {},
        [=](utility::event_atom, item_atom, const JsonStore &, const bool) {},
    };
}


void TimelineActor::init() {
    print_on_create(this, base_.name());
    print_on_exit(this, base_.name());

    event_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(event_group_);

    auto change_event_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(change_event_group_);

    auto history_uuid = Uuid::generate();
    auto history_ = spawn<history::HistoryMapActor<sys_time_point, JsonStore>>(history_uuid);
    link_to(history_);

    auto selection_actor_ = spawn<playhead::PlayheadSelectionActor>(
        "SubsetPlayheadSelectionActor", caf::actor_cast<caf::actor>(this));
    link_to(selection_actor_);

    set_down_handler([=](down_msg &msg) {
        // find in playhead list..
        for (auto it = std::begin(actors_); it != std::end(actors_); ++it) {
            // if a child dies we won't have enough information to recreate it.
            // we still need to report it up the chain though.

            if (msg.source == it->second) {
                demonitor(it->second);

                // if media..
                if (base_.remove_media(it->first)) {
                    send(event_group_, utility::event_atom_v, change_atom_v);
                    send(
                        event_group_,
                        utility::event_atom_v,
                        playlist::remove_media_atom_v,
                        UuidVector({it->first}));
                    base_.send_changed(event_group_, this);
                }

                actors_.erase(it);

                // remove from base.
                auto it = find_actor_addr(base_.item().children(), msg.source);

                if (it != base_.item().end()) {
                    auto jsn  = base_.item().erase(it);
                    auto more = base_.item().refresh();
                    if (not more.is_null())
                        jsn.insert(jsn.begin(), more.begin(), more.end());

                    send(event_group_, event_atom_v, item_atom_v, jsn, false);
                }
                break;
            }
        }
    });

    // update_edit_list_ = true;

    behavior_.assign(
        base_.make_set_name_handler(event_group_, this),
        base_.make_get_name_handler(),
        base_.make_last_changed_getter(),
        base_.make_last_changed_setter(event_group_, this),
        base_.make_last_changed_event_handler(event_group_, this),
        base_.make_get_uuid_handler(),
        base_.make_get_type_handler(),
        make_get_event_group_handler(event_group_),
        base_.make_get_detail_handler(this, event_group_),

        [=](broadcast::broadcast_down_atom, const caf::actor_addr &) {},
        [=](const group_down_msg & /*msg*/) {},

        [=](history::history_atom) -> UuidActor { return UuidActor(history_uuid, history_); },

        [=](link_media_atom, const bool force) -> result<bool> {
            auto rp = make_response_promise<bool>();

            if (actors_.empty()) {
                rp.deliver(true);
            } else {
                // pool direct children for state.
                fan_out_request<policy::select_all>(
                    map_value_to_vec(actors_),
                    infinite,
                    link_media_atom_v,
                    media_actors_,
                    force)
                    .await(
                        [=](std::vector<bool> items) mutable { rp.deliver(true); },
                        [=](error &err) mutable {
                            spdlog::warn(
                                "{} {} {}",
                                __PRETTY_FUNCTION__,
                                to_string(err),
                                base_.item().name());
                            rp.deliver(false);
                        });
            }

            return rp;
        },

        [=](link_media_atom, const UuidActorMap &media, const bool force) -> result<bool> {
            auto rp = make_response_promise<bool>();
            if (actors_.empty()) {
                rp.deliver(true);
            } else {
                // pool direct children for state.
                fan_out_request<policy::select_all>(
                    map_value_to_vec(actors_), infinite, link_media_atom_v, media, force)
                    .await(
                        [=](std::vector<bool> items) mutable { rp.deliver(true); },
                        [=](error &err) mutable { rp.deliver(err); });
            }

            return rp;
        },


        [=](active_range_atom, const FrameRange &fr) -> JsonStore {
            auto jsn = base_.item().set_active_range(fr);
            if (not jsn.is_null()) {
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
                anon_send(history_, history::log_atom_v, __sysclock_now(), jsn);
            }
            return jsn;
        },

        [=](item_flag_atom, const std::string &value) -> JsonStore {
            auto jsn = base_.item().set_flag(value);
            if (not jsn.is_null())
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
            return jsn;
        },

        [=](item_lock_atom, const bool value) -> JsonStore {
            auto jsn = base_.item().set_locked(value);
            if (not jsn.is_null())
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
            return jsn;
        },

        [=](item_name_atom, const std::string &value) -> JsonStore {
            auto jsn = base_.item().set_name(value);
            if (not jsn.is_null())
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
            return jsn;
        },

        [=](item_prop_atom, const utility::JsonStore &value) -> JsonStore {
            auto jsn = base_.item().set_prop(value);
            if (not jsn.is_null())
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
            return jsn;
        },

        [=](item_prop_atom,
            const utility::JsonStore &value,
            const std::string &path) -> JsonStore {
            auto prop = base_.item().prop();
            try {
                auto ptr = nlohmann::json::json_pointer(path);
                prop.at(ptr).update(value);
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }
            auto jsn = base_.item().set_prop(prop);
            if (not jsn.is_null())
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
            return jsn;
        },

        [=](item_prop_atom) -> JsonStore { return base_.item().prop(); },

        [=](available_range_atom, const FrameRange &fr) -> JsonStore {
            auto jsn = base_.item().set_available_range(fr);
            if (not jsn.is_null()) {
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
                anon_send(history_, history::log_atom_v, __sysclock_now(), jsn);
            }
            return jsn;
        },

        [=](active_range_atom) -> std::optional<utility::FrameRange> {
            return base_.item().active_range();
        },

        [=](available_range_atom) -> std::optional<utility::FrameRange> {
            return base_.item().available_range();
        },

        [=](trimmed_range_atom) -> utility::FrameRange { return base_.item().trimmed_range(); },

        [=](item_atom) -> Item { return base_.item(); },

        [=](plugin_manager::enable_atom, const bool value) -> JsonStore {
            auto jsn = base_.item().set_enabled(value);
            if (not jsn.is_null()) {
                send(event_group_, event_atom_v, item_atom_v, jsn, false);
                anon_send(history_, history::log_atom_v, sysclock::now(), jsn);
            }
            return jsn;
        },

        [=](item_atom, int index) -> result<Item> {
            if (static_cast<size_t>(index) >= base_.item().size()) {
                return make_error(xstudio_error::error, "Invalid index");
            }
            auto it = base_.item().cbegin();
            std::advance(it, index);
            return *it;
        },

        // search for item in children.
        [=](item_atom, const utility::Uuid &id) -> result<Item> {
            auto item = find_item(base_.item().children(), id);
            if (item)
                return **item;

            return make_error(xstudio_error::error, "Invalid uuid");
        },

        [=](utility::event_atom, utility::change_atom, const bool) {
            content_changed_ = false;
            // send(event_group_, event_atom_v, item_atom_v, base_.item());
            send(event_group_, utility::event_atom_v, change_atom_v);
            send(change_event_group_, utility::event_atom_v, utility::change_atom_v);
        },

        [=](utility::event_atom, utility::change_atom) {
            if (not content_changed_) {
                content_changed_ = true;
                delayed_send(
                    this,
                    std::chrono::milliseconds(50),
                    utility::event_atom_v,
                    change_atom_v,
                    true);
            }
        },

        // handle child change events.
        // [=](event_atom, item_atom, const Item &item) {
        //     // it's possibly one of ours.. so try and substitue the record
        //     if(base_.item().replace_child(item)) {
        //         base_.item().refresh();
        //         send(this, utility::event_atom_v, change_atom_v);
        //     }
        // },

        // handle child change events.
        [=](event_atom, item_atom, const JsonStore &update, const bool hidden) {
            if (base_.item().update(update)) {
                auto more = base_.item().refresh();
                if (not more.is_null()) {
                    more.insert(more.begin(), update.begin(), update.end());
                    send(event_group_, event_atom_v, item_atom_v, more, hidden);
                    if (not hidden)
                        anon_send(history_, history::log_atom_v, __sysclock_now(), more);

                    send(this, utility::event_atom_v, change_atom_v);
                    return;
                }
            }

            send(event_group_, event_atom_v, item_atom_v, update, hidden);
            if (not hidden)
                anon_send(history_, history::log_atom_v, __sysclock_now(), update);

            if (base_.item().has_dirty(update))
                send(this, utility::event_atom_v, change_atom_v);
        },

        // loop with timepoint
        [=](history::undo_atom, const sys_time_point &key) -> result<bool> {
            auto rp = make_response_promise<bool>();

            request(history_, infinite, history::undo_atom_v, key)
                .then(
                    [=](const JsonStore &hist) mutable {
                        rp.delegate(
                            caf::actor_cast<caf::actor>(this), history::undo_atom_v, hist);
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });
            return rp;
        },

        // loop with timepoint
        [=](history::redo_atom, const sys_time_point &key) -> result<bool> {
            auto rp = make_response_promise<bool>();

            request(history_, infinite, history::redo_atom_v, key)
                .then(
                    [=](const JsonStore &hist) mutable {
                        rp.delegate(
                            caf::actor_cast<caf::actor>(this), history::redo_atom_v, hist);
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });
            return rp;
        },

        [=](history::undo_atom, const utility::sys_time_duration &duration) -> result<bool> {
            auto rp = make_response_promise<bool>();
            request(history_, infinite, history::undo_atom_v, duration)
                .then(
                    [=](const std::vector<JsonStore> &hist) mutable {
                        auto count = std::make_shared<size_t>(0);
                        for (const auto &h : hist) {
                            request(
                                caf::actor_cast<caf::actor>(this),
                                infinite,
                                history::undo_atom_v,
                                h)
                                .then(
                                    [=](const bool) mutable {
                                        (*count)++;
                                        if (*count == hist.size())
                                            rp.deliver(true);
                                    },
                                    [=](const error &err) mutable {
                                        (*count)++;
                                        if (*count == hist.size())
                                            rp.deliver(true);
                                    });
                        }
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });
            return rp;
        },

        [=](history::redo_atom, const utility::sys_time_duration &duration) -> result<bool> {
            auto rp = make_response_promise<bool>();
            request(history_, infinite, history::redo_atom_v, duration)
                .then(
                    [=](const std::vector<JsonStore> &hist) mutable {
                        auto count = std::make_shared<size_t>(0);
                        for (const auto &h : hist) {
                            request(
                                caf::actor_cast<caf::actor>(this),
                                infinite,
                                history::redo_atom_v,
                                h)
                                .then(
                                    [=](const bool) mutable {
                                        (*count)++;
                                        if (*count == hist.size())
                                            rp.deliver(true);
                                    },
                                    [=](const error &err) mutable {
                                        (*count)++;
                                        if (*count == hist.size())
                                            rp.deliver(true);
                                    });
                        }
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });
            return rp;
        },

        [=](history::undo_atom) -> result<bool> {
            auto rp = make_response_promise<bool>();
            request(history_, infinite, history::undo_atom_v)
                .then(
                    [=](const JsonStore &hist) mutable {
                        rp.delegate(
                            caf::actor_cast<caf::actor>(this), history::undo_atom_v, hist);
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });
            return rp;
        },

        [=](history::redo_atom) -> result<bool> {
            auto rp = make_response_promise<bool>();
            request(history_, infinite, history::redo_atom_v)
                .then(
                    [=](const JsonStore &hist) mutable {
                        rp.delegate(
                            caf::actor_cast<caf::actor>(this), history::redo_atom_v, hist);
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });
            return rp;
        },

        [=](history::undo_atom, const JsonStore &hist) -> result<bool> {
            auto rp = make_response_promise<bool>();

            base_.item().undo(hist);

            auto inverted = R"([])"_json;
            for (const auto &i : hist) {
                auto ev    = R"({})"_json;
                ev["redo"] = i.at("undo");
                ev["undo"] = i.at("redo");
                inverted.emplace_back(ev);
            }

            // send(event_group_, event_atom_v, item_atom_v, JsonStore(inverted), true);

            if (not actors_.empty()) {
                // push to children..
                fan_out_request<policy::select_all>(
                    map_value_to_vec(actors_), infinite, history::undo_atom_v, hist)
                    .await(
                        [=](std::vector<bool> updated) mutable {
                            anon_send(this, link_media_atom_v, media_actors_, false);
                            send(
                                event_group_,
                                event_atom_v,
                                item_atom_v,
                                JsonStore(inverted),
                                true);
                            rp.deliver(true);
                        },
                        [=](error &err) mutable { rp.deliver(std::move(err)); });
            } else {
                send(event_group_, event_atom_v, item_atom_v, JsonStore(inverted), true);
                rp.deliver(true);
            }
            return rp;
        },

        [=](history::redo_atom, const JsonStore &hist) -> result<bool> {
            auto rp = make_response_promise<bool>();
            base_.item().redo(hist);

            // send(event_group_, event_atom_v, item_atom_v, hist, true);

            if (not actors_.empty()) {
                // push to children..
                fan_out_request<policy::select_all>(
                    map_value_to_vec(actors_), infinite, history::redo_atom_v, hist)
                    .await(
                        [=](std::vector<bool> updated) mutable {
                            rp.deliver(true);
                            anon_send(this, link_media_atom_v, media_actors_, false);

                            send(event_group_, event_atom_v, item_atom_v, hist, true);
                        },
                        [=](error &err) mutable { rp.deliver(std::move(err)); });
            } else {
                send(event_group_, event_atom_v, item_atom_v, hist, true);
                rp.deliver(true);
            }

            return rp;
        },

        [=](bake_atom, const UuidVector &uuids) -> result<UuidActor> {
            auto rp = make_response_promise<UuidActor>();

            bake(rp, utility::UuidSet(uuids.begin(), uuids.end()));

            return rp;
        },

        [=](bake_atom, const FrameRate &time) -> result<ResolvedItem> {
            auto ri =
                base_.item().resolve_time(time, media::MediaType::MT_IMAGE, base_.focus_list());
            if (ri)
                return *ri;

            return make_error(xstudio_error::error, "No clip resolved");
        },

        [=](insert_item_atom,
            const int index,
            const UuidActorVector &uav) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();

            if (not base_.item().empty() or uav.size() > 1)
                rp.deliver(make_error(xstudio_error::error, "Only one child allowed"));
            else
                insert_items(index, uav, rp);
            return rp;
        },

        [=](insert_item_atom,
            const utility::Uuid &before_uuid,
            const UuidActorVector &uav) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();

            if (not base_.item().empty() or uav.size() > 1)
                rp.deliver(make_error(xstudio_error::error, "Only one child allowed"));
            else {

                auto index = base_.item().size();
                // find index. for uuid
                if (not before_uuid.is_null()) {
                    auto it = find_uuid(base_.item().children(), before_uuid);
                    if (it == base_.item().end())
                        rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));
                    else
                        index = std::distance(base_.item().begin(), it);
                }

                if (rp.pending())
                    insert_items(index, uav, rp);
            }

            return rp;
        },

        [=](remove_item_atom,
            const int index,
            const bool) -> result<std::pair<JsonStore, std::vector<Item>>> {
            auto rp = make_response_promise<std::pair<JsonStore, std::vector<Item>>>();
            remove_items(index, 1, rp);
            return rp;
        },

        [=](remove_item_atom, const int index, const int count, const bool)
            -> result<std::pair<JsonStore, std::vector<Item>>> {
            auto rp = make_response_promise<std::pair<JsonStore, std::vector<Item>>>();
            remove_items(index, count, rp);
            return rp;
        },

        // delegate to deep child..
        [=](remove_item_atom, const utility::Uuid &uuid, const bool add_gap, const bool)
            -> result<std::pair<JsonStore, std::vector<Item>>> {
            auto rp = make_response_promise<std::pair<JsonStore, std::vector<Item>>>();

            auto actor = find_parent_actor(base_.item(), uuid);

            if (not actor)
                rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));
            else
                rp.delegate(actor, remove_item_atom_v, uuid, add_gap);

            return rp;
        },

        [=](remove_item_atom,
            const utility::Uuid &uuid,
            const bool) -> result<std::pair<JsonStore, std::vector<Item>>> {
            auto rp = make_response_promise<std::pair<JsonStore, std::vector<Item>>>();

            auto it = find_uuid(base_.item().children(), uuid);

            if (it == base_.item().end())
                rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));

            if (rp.pending())
                remove_items(std::distance(base_.item().begin(), it), 1, rp);

            return rp;
        },

        [=](erase_item_atom, const int index, const bool) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();
            erase_items(index, 1, rp);
            return rp;
        },

        [=](erase_item_atom, const int index, const int count, const bool)
            -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();
            erase_items(index, count, rp);
            return rp;
        },

        // delegate to deep child..
        [=](erase_item_atom, const utility::Uuid &uuid, const bool add_gap, const bool)
            -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();

            auto actor = find_parent_actor(base_.item(), uuid);

            if (not actor)
                rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));
            else
                rp.delegate(actor, erase_item_atom_v, uuid, add_gap);

            return rp;
        },

        [=](erase_item_atom, const utility::Uuid &uuid, const bool) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();

            auto it = find_uuid(base_.item().children(), uuid);

            if (it == base_.item().end())
                rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));

            if (rp.pending())
                erase_items(std::distance(base_.item().begin(), it), 1, rp);

            return rp;
        },

        // emulate subset
        [=](playlist::sort_by_media_display_info_atom,
            const int info_set_idx,
            const int info_item_idx,
            const bool ascending) {
            sort_by_media_display_info(info_set_idx, info_item_idx, ascending);
        },

        [=](media::get_edit_list_atom, media::MediaType, const Uuid &) -> utility::EditList {
            // Edit list actor (from A/B compare in PlaheadActor) sends this
            // message, we will return empty edit list as getting Timelines to
            // construct EditLists seems pointless at this stage and instead we
            // will get rid of EditListActor
            return utility::EditList();
        },

        [=](media::source_offset_frames_atom) -> int {
            // needed when retime actor wraps a timeline
            return 0;
        },

        [=](media::source_offset_frames_atom, const int) -> bool {
            // needed when retime actor wraps a timeline
            return false;
        },

        [=](timeline::duration_atom, const timebase::flicks &new_duration) -> bool {
            // attempt by playhead to force trim the duration (to support compare
            // modes for sources of different lenght). Here we ignore it.
            return false;
        },

        [=](media::get_edit_list_atom, const Uuid &uuid) -> result<utility::EditList> {
            std::vector<caf::actor> actors;
            for (const auto &i : base_.media())
                actors.push_back(actors_[i]);

            if (not actors.empty()) {
                auto rp = make_response_promise<utility::EditList>();

                fan_out_request<policy::select_all>(
                    actors, infinite, media::get_edit_list_atom_v, Uuid())
                    .then(
                        [=](std::vector<utility::EditList> sections) mutable {
                            utility::EditList ordered_sections;
                            for (const auto &i : base_.media()) {
                                for (const auto &ii : sections) {
                                    const auto &[ud, rt, tc] = ii.section_list()[0];
                                    if (ud == i) {
                                        if (uuid.is_null())
                                            ordered_sections.push_back(ii.section_list()[0]);
                                        else
                                            ordered_sections.push_back({uuid, rt, tc});
                                        break;
                                    }
                                }
                            }
                            rp.deliver(ordered_sections);
                        },
                        [=](error &err) mutable { rp.deliver(std::move(err)); });

                return rp;
            }

            return result<utility::EditList>(utility::EditList());
        },

        [=](media::get_media_pointer_atom,
            const media::MediaType media_type,
            const int logical_frame) -> result<media::AVFrameID> {
            // get actors attached to our media..
            if (not base_.empty()) {
                auto rp = make_response_promise<media::AVFrameID>();
                deliver_media_pointer(logical_frame, media_type, rp);
                return rp;
            }

            return result<media::AVFrameID>(make_error(xstudio_error::error, "No media"));
        },


        [=](playlist::get_media_uuid_atom) -> UuidVector { return base_.media_vector(); },

        [=](playlist::add_media_atom,
            const UuidActorVector &uav,
            const Uuid &before_uuid,
            const bool duplicate) -> bool {
            // add internally
            for (const auto &i : uav) {
                add_media(i.actor(), i.uuid(), before_uuid);
            }

            return true;
        },

        [=](playlist::add_media_atom,
            const UuidActorVector &uav,
            const Uuid &before_uuid) -> result<bool> {
            auto rp = make_response_promise<bool>();

            // add internally
            for (const auto &i : uav) {
                add_media(i.actor(), i.uuid(), before_uuid);
            }

            // dispatch to playlist
            request(
                caf::actor_cast<caf::actor>(playlist_),
                infinite,
                playlist::add_media_atom_v,
                uav,
                Uuid())
                .then(
                    [=](const bool) mutable {
                        rp.deliver(true);

                        anon_send(
                            caf::actor_cast<caf::actor>(playlist_),
                            media_hook::gather_media_sources_atom_v,
                            vector_to_uuid_vector(uav));

                        // just one vent to trigger rebuild ?
                        send(
                            event_group_,
                            utility::event_atom_v,
                            playlist::add_media_atom_v,
                            UuidActorVector({UuidActor(uav[0].uuid(), uav[0].actor())}));

                        base_.send_changed(event_group_, this);
                        send(event_group_, utility::event_atom_v, change_atom_v);
                        send(
                            change_event_group_, utility::event_atom_v, utility::change_atom_v);
                    },
                    [=](const caf::error &err) mutable { rp.deliver(err); });

            return rp;
        },

        [=](session::media_rate_atom atom) {
            delegate(caf::actor_cast<caf::actor>(playlist_), atom);
        },

        [=](playlist::add_media_atom,
            const UuidActor &ua,
            const Uuid &before_uuid) -> result<UuidActor> {
            auto rp = make_response_promise<UuidActor>();
            add_media(ua, before_uuid, rp);
            return rp;
        },

        [=](playlist::add_media_atom,
            const Uuid &uuid,
            const Uuid &before_uuid) -> result<bool> {
            // get actor from playlist..
            auto rp = make_response_promise<bool>();

            request(
                caf::actor_cast<actor>(playlist_), infinite, playlist::get_media_atom_v, uuid)
                .then(
                    [=](caf::actor actor) mutable {
                        rp.delegate(
                            caf::actor_cast<caf::actor>(this),
                            playlist::add_media_atom_v,
                            uuid,
                            actor,
                            before_uuid);
                        // add_media(actor, uuid, before_uuid);
                        // send(event_group_, utility::event_atom_v, change_atom_v);
                        // send(change_event_group_, utility::event_atom_v,
                        // utility::change_atom_v); send(
                        //     event_group_,
                        //     utility::event_atom_v,
                        //     playlist::add_media_atom_v,
                        //     UuidActorVector({UuidActor(uuid, actor)}));
                        // base_.send_changed(event_group_, this);
                        // rp.deliver(true);
                    },
                    [=](error &err) mutable {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                        rp.deliver(false);
                    });

            return rp;
        },

        [=](playlist::add_media_atom,
            const Uuid &uuid,
            const caf::actor &actor,
            const Uuid &before_uuid) -> bool {
            try {
                add_media(actor, uuid, before_uuid);
                send(
                    event_group_,
                    utility::event_atom_v,
                    playlist::add_media_atom_v,
                    UuidActorVector({UuidActor(uuid, actor)}));
                base_.send_changed(event_group_, this);
                send(event_group_, utility::event_atom_v, change_atom_v);
                send(change_event_group_, utility::event_atom_v, utility::change_atom_v);
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }
            return true;
        },

        [=](playlist::add_media_atom,
            caf::actor actor,
            const Uuid &before_uuid) -> result<bool> {
            auto rp = make_response_promise<bool>();

            caf::scoped_actor sys(system());
            try {
                // get uuid..
                Uuid uuid = request_receive<Uuid>(*sys, actor, utility::uuid_atom_v);
                // check playlist owns it..
                request_receive<caf::actor>(
                    *sys,
                    caf::actor_cast<caf::actor>(playlist_),
                    playlist::get_media_atom_v,
                    uuid);

                rp.delegate(
                    caf::actor_cast<caf::actor>(this),
                    playlist::add_media_atom_v,
                    uuid,
                    actor,
                    before_uuid);
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                rp.deliver(false);
            }

            return rp;
        },

        [=](playlist::add_media_atom atom,
            UuidActor ua, // the MediaActor
            const utility::UuidList &final_ordered_uuid_list,
            utility::Uuid before) -> result<UuidActor> {
            auto rp = make_response_promise<UuidActor>();
            // This handler lets us build a playlist of a given order, even when
            // we add the media in an out of order way. We generate Uuids for the
            // MediaActors that will be used to build the playlist *before* the
            // actual MediaActors are built and added - we pass in the ordered
            // list of Uuids when adding each MediaActor so we can insert it
            // into the correct point as the playlist is being built.
            //
            // This is used in ShotgunDataSourceActor, for example

            const utility::UuidList media = base_.media();

            // get an iterator to the uuid of the next MediaActor item that has
            // been (or will be) added to this playlist
            auto next = std::find(
                final_ordered_uuid_list.begin(), final_ordered_uuid_list.end(), ua.uuid());
            if (next != final_ordered_uuid_list.end())
                next++;

            while (next != final_ordered_uuid_list.end()) {

                // has 'next' already been added to this playlist?
                auto q = std::find(media.begin(), media.end(), *next);
                if (q != media.end()) {
                    // yes - we know where to insert the incoming MediaActor
                    before = *q;
                    break;
                }
                // keep looking
                next++;
            }

            // Note we can't use delegate(this, add_media_atom_v, ua, before)
            // to enact the adding, because it might happen *after* we get
            // another of these add_media messages which would then mess up the
            // ordering algorithm
            add_media(ua, before, rp);

            return rp;
        },

        [=](playlist::get_next_media_atom,
            const utility::Uuid &after_this_uuid,
            int skip_by) -> result<UuidActor> {
            const utility::UuidList media = base_.media();

            if (skip_by > 0) {
                auto i = std::find(media.begin(), media.end(), after_this_uuid);
                if (i == media.end()) {
                    // not found!
                    return make_error(
                        xstudio_error::error,
                        fmt::format(
                            "playlist::get_next_media_atom called with uuid that is not in "
                            "timeline {}",
                            to_string(after_this_uuid)));
                }
                while (skip_by--) {
                    i++;
                    if (i == media.end()) {
                        i--;
                        break;
                    }
                }
                if (media_actors_.count(*i))
                    return UuidActor(*i, media_actors_[*i]);

            } else {
                auto i = std::find(media.rbegin(), media.rend(), after_this_uuid);
                if (i == media.rend()) {
                    // not found!
                    return make_error(
                        xstudio_error::error,
                        fmt::format(
                            "playlist::get_next_media_atom called with uuid that is not in "
                            "playlist",
                            to_string(after_this_uuid)));
                }
                while (skip_by++) {
                    i++;
                    if (i == media.rend()) {
                        i--;
                        break;
                    }
                }

                if (media_actors_.count(*i))
                    return UuidActor(*i, media_actors_[*i]);
            }

            return make_error(
                xstudio_error::error,
                fmt::format(
                    "playlist::get_next_media_atom called with uuid for which no media actor "
                    "exists {}",
                    to_string(after_this_uuid)));
        },

        [=](playlist::create_playhead_atom, const int index) -> result<UuidActor> {
            // aux playheads where index >= 0

            auto rp = make_response_promise<UuidActor>();
            if (!index) {
                rp.delegate(
                    caf::actor_cast<caf::actor>(this), playlist::create_playhead_atom_v);
                return rp;
            }

            if (aux_playheads_.find(index) != aux_playheads_.end()) {
                rp.deliver(aux_playheads_[index]);
                return rp;
            }

            auto uuid = utility::Uuid::generate();

            auto playhead_actor = spawn<playhead::PlayheadActor>(
                fmt::format("Timeline Aux Playhead {}", index), selection_actor_, uuid);

            link_to(playhead_actor);

            anon_send(playhead_actor, playhead::playhead_rate_atom_v, base_.rate());

            aux_playheads_[index] = UuidActor(uuid, playhead_actor);
            rp.deliver(aux_playheads_[index]);
            return rp;
        },

        [=](playlist::create_playhead_atom) -> UuidActor {
            if (playhead_)
                return playhead_;

            auto uuid = utility::Uuid::generate();

            /*auto actor = spawn<playhead::PlayheadActor>(
                std::string("Timeline Playhead"), selection_actor_, uuid);*/

            // N.B. for now we're not using the 'selection_actor_' as this
            // feeds the playhead a list of selected media which the playhead
            // will play. It will ignore this timeline completely if we do that.
            // We want to play this timeline, not the media in the timeline
            // that is selected.
            auto playhead_actor = spawn<playhead::PlayheadActor>(
                std::string("Timeline Playhead"), caf::actor(), uuid);

            link_to(playhead_actor);

            anon_send(playhead_actor, playhead::playhead_rate_atom_v, base_.rate());

            // now make this timeline the (only) source for the playhead
            anon_send(
                playhead_actor,
                playhead::source_atom_v,
                std::vector<caf::actor>({caf::actor_cast<caf::actor>(this)}));

            playhead_ = UuidActor(uuid, playhead_actor);
            return playhead_;
        },

        [=](playlist::get_playhead_atom) {
            delegate(caf::actor_cast<caf::actor>(this), playlist::create_playhead_atom_v);
        },

        [=](playlist::get_playhead_atom, const int index) {
            delegate(
                caf::actor_cast<caf::actor>(this), playlist::create_playhead_atom_v, index);
        },

        [=](playlist::get_change_event_group_atom) -> caf::actor {
            return change_event_group_;
        },

        [=](playlist::get_media_atom, const bool) -> result<std::vector<ContainerDetail>> {
            std::vector<caf::actor> actors;
            // only media actors..
            for (const auto &i : base_.media())
                actors.push_back(media_actors_[i]);

            if (not actors.empty()) {
                auto rp = make_response_promise<std::vector<ContainerDetail>>();
                // collect media data..

                fan_out_request<policy::select_all>(actors, infinite, utility::detail_atom_v)
                    .then(
                        [=](const std::vector<ContainerDetail> details) mutable {
                            std::vector<ContainerDetail> reordered_details;

                            for (const auto &i : base_.media()) {
                                for (const auto &ii : details) {
                                    if (ii.uuid_ == i) {
                                        reordered_details.push_back(ii);
                                        break;
                                    }
                                }
                            }

                            rp.deliver(reordered_details);
                        },
                        [=](error &err) mutable { rp.deliver(std::move(err)); });

                return rp;
            }
            std::vector<ContainerDetail> result;
            return result;
        },

        [=](playlist::get_media_atom, const Uuid &uuid) -> result<caf::actor> {
            if (base_.contains_media(uuid))
                return media_actors_[uuid];
            return make_error(xstudio_error::error, "Invalid uuid");
        },

        [=](playlist::get_media_atom) -> UuidActorVector {
            UuidActorVector result;

            for (const auto &i : base_.media())
                result.emplace_back(UuidActor(i, media_actors_[i]));

            return result;
        },

        [=](playlist::selection_actor_atom) -> caf::actor { return selection_actor_; },

        [=](playlist::move_media_atom atom, const Uuid &uuid, const Uuid &uuid_before) {
            delegate(
                actor_cast<caf::actor>(this), atom, utility::UuidVector({uuid}), uuid_before);
        },

        [=](playlist::move_media_atom atom,
            const UuidList &media_uuids,
            const Uuid &uuid_before) {
            delegate(
                actor_cast<caf::actor>(this),
                atom,
                utility::UuidVector(media_uuids.begin(), media_uuids.end()),
                uuid_before);
        },

        [=](playlist::move_media_atom,
            const UuidVector &media_uuids,
            const Uuid &uuid_before) -> bool {
            bool result = false;
            for (auto uuid : media_uuids) {
                result |= base_.move_media(uuid, uuid_before);
            }
            if (result) {
                base_.send_changed(event_group_, this);
                send(
                    event_group_,
                    utility::event_atom_v,
                    playlist::move_media_atom_v,
                    media_uuids,
                    uuid_before);
                send(event_group_, utility::event_atom_v, change_atom_v);
                send(change_event_group_, utility::event_atom_v, utility::change_atom_v);
            }
            return result;
        },

        [=](playlist::remove_media_atom atom, const utility::UuidList &uuids) {
            delegate(
                actor_cast<caf::actor>(this),
                atom,
                utility::UuidVector(uuids.begin(), uuids.end()));
        },

        [=](playlist::remove_media_atom atom, const Uuid &uuid) {
            delegate(actor_cast<caf::actor>(this), atom, utility::UuidVector({uuid}));
        },

        [=](media::current_media_source_atom)
            -> caf::result<std::vector<std::pair<UuidActor, std::pair<UuidActor, UuidActor>>>> {
            auto rp = make_response_promise<
                std::vector<std::pair<UuidActor, std::pair<UuidActor, UuidActor>>>>();
            if (not media_actors_.empty()) {
                fan_out_request<policy::select_all>(
                    map_value_to_vec(media_actors_),
                    infinite,
                    media::current_media_source_atom_v)
                    .then(
                        [=](const std::vector<
                            std::pair<UuidActor, std::pair<UuidActor, UuidActor>>>
                                details) mutable { rp.deliver(details); },
                        [=](error &err) mutable { rp.deliver(std::move(err)); });
            } else {
                rp.deliver(
                    std::vector<std::pair<UuidActor, std::pair<UuidActor, UuidActor>>>());
            }
            return rp;
        },

        [=](playlist::remove_media_atom, const utility::UuidVector &uuids) -> bool {
            // this needs to propergate to children somehow..
            utility::UuidVector removed;

            for (const auto &uuid : uuids) {
                if (media_actors_.count(uuid) and remove_media(media_actors_[uuid], uuid)) {
                    removed.push_back(uuid);

                    for (const auto &i : find_media_clips(base_.children(), uuid)) {
                        // find parent actor of clip and remove..
                        auto pa = find_parent_actor(base_.item(), i);
                        if (pa)
                            anon_send(pa, erase_item_atom_v, i, true);
                    }
                }
            }

            if (not removed.empty()) {
                send(event_group_, utility::event_atom_v, change_atom_v);
                send(
                    event_group_,
                    utility::event_atom_v,
                    playlist::remove_media_atom_v,
                    removed);
                send(change_event_group_, utility::event_atom_v, utility::change_atom_v);
                base_.send_changed(event_group_, this);
            }
            return not removed.empty();
        },


        // // code for playhead// get edit_list for all tracks/stacks..// this is temporary,
        // it'll
        // // need heavy changes..// also this only returns edit_lists for images, audio may be
        // // different..
        // [=](media::get_edit_list_atom, const Uuid &uuid) -> result<utility::EditList> {
        //     if (update_edit_list_) {
        //         std::vector<caf::actor> actors;
        //         for (const auto &i : base_.tracks())
        //             actors.push_back(actors_[i]);

        //         if (not actors.empty()) {
        //             auto rp = make_response_promise<utility::EditList>();

        //             fan_out_request<policy::select_all>(
        //                 actors, infinite, media::get_edit_list_atom_v, Uuid())
        //                 .await(
        //                     [=](std::vector<utility::EditList> sections) mutable {
        //                         edit_list_.clear();
        //                         for (const auto &i : base_.tracks()) {
        //                             for (const auto &ii : sections) {
        //                                 for (const auto &section : ii.section_list()) {
        //                                     const auto &[ud, rt, tc] = section;
        //                                     if (ud == i) {
        //                                         if (uuid.is_null())
        //                                             edit_list_.push_back(section);
        //                                         else
        //                                             edit_list_.push_back({uuid, rt, tc});
        //                                     }
        //                                 }
        //                             }
        //                         }
        //                         update_edit_list_ = false;
        //                         rp.deliver(edit_list_);
        //                     },
        //                     [=](error &err) mutable { rp.deliver(std::move(err)); });

        //             return rp;
        //         } else {
        //             edit_list_.clear();
        //             update_edit_list_ = false;
        //         }
        //     }

        //     return result<utility::EditList>(edit_list_);
        // },

        // [=](media::get_media_pointer_atom,
        //     const int logical_frame) -> result<media::AVFrameID> {
        //     if (base_.empty())
        //         return result<media::AVFrameID>(make_error(xstudio_error::error, "No
        //         media"));

        //     auto rp = make_response_promise<media::AVFrameID>();
        //     if (update_edit_list_) {
        //         request(actor_cast<caf::actor>(this), infinite, media::get_edit_list_atom_v)
        //             .then(
        //                 [=](const utility::EditList &) mutable {
        //                     deliver_media_pointer(logical_frame, rp);
        //                 },
        //                 [=](error &err) mutable { rp.deliver(std::move(err)); });
        //     } else {
        //         deliver_media_pointer(logical_frame, rp);
        //     }

        //     return rp;
        // },

        // [=](start_time_atom) -> utility::FrameRateDuration { return base_.start_time(); },

        // [=](utility::clear_atom) -> bool {
        //     base_.clear();
        //     for (const auto &i : actors_) {
        //         // this->leave(i.second);
        //         unlink_from(i.second);
        //         send_exit(i.second, caf::exit_reason::user_shutdown);
        //     }
        //     actors_.clear();
        //     return true;
        // },

        // [=](utility::event_atom, utility::change_atom) {
        //     update_edit_list_ = true;
        //     send(event_group_, utility::event_atom_v, utility::change_atom_v);
        // },

        // [=](utility::event_atom, utility::name_atom, const std::string & /*name*/) {},

        // [=](utility::rate_atom) -> FrameRate { return base_.rate(); },

        // [=](utility::rate_atom, const FrameRate &rate) { base_.set_rate(rate); },

        // this operation isn't relevant ?


        // [=](playlist::create_playhead_atom) -> UuidActor {
        //     if (playhead_)
        //         return playhead_;
        //     auto uuid  = utility::Uuid::generate();
        //     auto actor = spawn<playhead::PlayheadActor>(
        //         std::string("Timeline Playhead"), caf::actor_cast<caf::actor>(this), uuid);
        //     link_to(actor);
        //     playhead_ = UuidActor(uuid, actor);

        //     anon_send(actor, playhead::playhead_rate_atom_v, base_.rate());

        //     // this pushes this actor to the playhead as the 'source' that the
        //     // playhead will play
        //     send(
        //         actor,
        //         utility::event_atom_v,
        //         playhead::source_atom_v,
        //         std::vector<caf::actor>{caf::actor_cast<caf::actor>(this)});

        //     return playhead_;
        // },

        // emulate subset.

        // [=](playlist::selection_actor_atom) -> caf::actor {
        //     return caf::actor_cast<caf::actor>(this);
        // },

        [=](duplicate_atom) -> result<UuidActor> {
            auto rp = make_response_promise<UuidActor>();

            try {
                // clone ourself..
                caf::scoped_actor sys(system());

                JsonStore jsn;
                auto dup = base_.duplicate();
                dup.item().clear();

                jsn["base"]   = dup.serialise();
                jsn["actors"] = {};
                auto actor    = spawn<TimelineActor>(jsn, caf::actor());

                auto hactor = request_receive<UuidActor>(*sys, actor, history::history_atom_v);
                anon_send(hactor.actor(), plugin_manager::enable_atom_v, false);

                for (const auto &i : base_.children()) {
                    auto ua = request_receive<UuidActor>(
                        *sys, actors_[i.uuid()], utility::duplicate_atom_v);
                    request_receive<JsonStore>(
                        *sys, actor, insert_item_atom_v, -1, UuidActorVector({ua}));
                }

                // enable history
                anon_send(hactor.actor(), plugin_manager::enable_atom_v, true);

                rp.deliver(UuidActor(dup.uuid(), actor));

            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                rp.deliver(make_error(xstudio_error::error, err.what()));
            }

            return rp;
        },

        [=](timeline::focus_atom) -> UuidVector {
            auto tmp = base_.focus_list();
            return UuidVector(tmp.begin(), tmp.end());
        },

        [=](timeline::focus_atom, const UuidVector &list) {
            base_.set_focus_list(list);
            // both ?
            send(event_group_, utility::event_atom_v, change_atom_v);
            send(change_event_group_, utility::event_atom_v, utility::change_atom_v);
        },

        [=](playhead::source_atom) -> caf::actor {
            return caf::actor_cast<caf::actor>(playlist_);
        },

        // set source (playlist), triggers relinking
        [=](playhead::source_atom,
            caf::actor playlist,
            const UuidUuidMap &swap) -> result<bool> {
            // spdlog::warn("playhead::source_atom old {} new {}", to_string(playlist_),
            // to_string(playlist)); for(const auto &i: swap)
            //     spdlog::warn("{} {}", to_string(i.first), to_string(i.second));

            auto rp = make_response_promise<bool>();

            for (const auto &i : media_actors_)
                demonitor(i.second);
            media_actors_.clear();

            playlist_ = caf::actor_cast<actor_addr>(playlist);

            request(playlist, infinite, playlist::get_media_atom_v)
                .then(
                    [=](const std::vector<UuidActor> &media) mutable {
                        // build map
                        UuidActorMap amap;
                        for (const auto &i : media)
                            amap[i.uuid()] = i.actor();

                        bool clean = false;
                        while (not clean) {
                            clean = true;
                            for (const auto &i : base_.media()) {
                                auto ii = (swap.count(i) ? swap.at(i) : i);
                                if (not amap.count(ii)) {
                                    spdlog::error(
                                        "Failed to find media in playlist {}", to_string(ii));
                                    base_.remove_media(i);
                                    clean = false;
                                    break;
                                }
                            }
                        }
                        // link
                        for (const auto &i : base_.media()) {
                            auto ii = (swap.count(i) ? swap.at(i) : i);
                            if (ii != i) {
                                base_.swap_media(i, ii);
                            }
                            media_actors_[ii] = amap[ii];
                            monitor(amap[ii]);
                        }

                        // for(const auto &i: actors_)
                        //     spdlog::warn("{} {}", to_string(i.first),to_string(i.second));

                        // timeline has only one child, the stack (we hope)
                        // relink all clips..
                        fan_out_request<policy::select_all>(
                            map_value_to_vec(actors_),
                            infinite,
                            playhead::source_atom_v,
                            swap,
                            media_actors_)
                            .await(
                                [=](std::vector<bool> items) mutable {
                                    base_.send_changed(event_group_, this);
                                    rp.deliver(true);
                                },
                                [=](error &err) mutable {
                                    spdlog::warn(
                                        "{} {} {}",
                                        __PRETTY_FUNCTION__,
                                        to_string(err),
                                        base_.item().name());
                                    rp.deliver(false);
                                });
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });

            return rp;
        },

        [=](duration_atom, const int) {},

        /*
                    // FOR TED
                    // iterate over direct children of stack item, and only return indexs of
           audio tracks that are enabled. auto audio_indexes =
           find_indexes(base_.item().front().children(), ItemType::IT_AUDIO_TRACK, true);
                    // resolve frames, using indexes from above.
                    if(not audio_indexes.empty()) {
                        // get first audio index
                        auto audio_index = audio_indexes[0];

                        // get access to the audio track item.
                        auto track_it = base_.item().front().item_at_index(audio_index);

                        // if it's valid (which it should be)
                        // resolve frame
                        // may return {} if no clip.
                        // focus list won't work correctly doing it this way.
                        // that would require the resolve_time function to return a vector, and
           not be used on individal tracks.

                        if(track_it) {
                            auto ii = (*track_it)->resolve_time(
                                            FrameRate(0),
                                            media::MediaType::MT_AUDIO,
                                            base_.focus_list());
                        }
                    }
        */

        [=](media::get_media_pointers_atom atom,
            const media::MediaType media_type,
            const utility::TimeSourceMode tsm,
            const utility::FrameRate &override_rate) -> caf::result<media::FrameTimeMap> {
            // This is required by SubPlayhead actor to make the timeline
            // playable.

            auto rp = make_response_promise<media::FrameTimeMap>();

            if (!base_.item().available_range()) {
                rp.deliver(media::FrameTimeMap());
                return rp;
            }

            // Should this be trimmed_range, active_range or available_range or
            // something else?
            const int start_frame =
                (*base_.item().available_range()).frame_start().frames(override_rate);
            const int end_frame =
                start_frame +
                (*base_.item().available_range()).frame_duration().frames(override_rate);

            // request the sequential AVFrameIDs for this timeline
            request(
                caf::actor_cast<caf::actor>(this),
                infinite,
                atom,
                media_type,
                media::LogicalFrameRanges({{
                    start_frame,
                    end_frame,
                }}),
                override_rate)
                .then(
                    [=](const media::AVFrameIDs &frame_ids) mutable {
                        auto time_point = timebase::flicks(0);
                        media::FrameTimeMap reslt;
                        int logical_frame = 0;
                        for (const auto &frame_id : frame_ids) {
                            auto frame_id_cpy = std::make_shared<media::AVFrameID>(*frame_id);
                            frame_id_cpy->playhead_logical_frame_ = logical_frame++;

                            // use the base rate to set the frame rate - this
                            // could be varied within this function if it fits
                            // with the timeline model. For example, supporting
                            // media of different frame rates in one timeline?
                            frame_id_cpy->rate_ = base_.rate();
                            reslt[time_point]   = frame_id_cpy;

                            // This is where the frame rate for the current
                            // frame is actually applied. We can increment
                            // by anything which allows playheads to play
                            // media of different rates.
                            time_point += frame_id_cpy->rate_.to_flicks();
                        }
                        rp.deliver(reslt);
                    },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });

            return rp;
        },

        [=](media::get_media_pointers_atom atom,
            const media::MediaType media_type,
            const media::LogicalFrameRanges &ranges,
            const FrameRate &override_rate) -> caf::result<media::AVFrameIDs> {
            auto num_frames = 0;
            for (const auto &i : ranges)
                num_frames += (i.second - i.first) + 1;


            auto result = std::make_shared<media::AVFrameIDs>(num_frames);
            auto count  = std::make_shared<int>();
            *count      = 0;
            // spdlog::warn("{} {} {} {}", media_type, num_frames, start_frame,
            // override_rate.to_fps());

            caf::scoped_actor sys(system());

            auto item_tp = std::vector<std::optional<ResolvedItem>>();
            item_tp.reserve(num_frames);

            // spdlog::stopwatch sw;
            for (const auto &r : ranges) {
                for (auto i = r.first; i <= r.second; i++) {
                    auto ii = base_.item().resolve_time(
                        FrameRate(i * override_rate.to_flicks()),
                        media_type,
                        base_.focus_list());
                    if (ii) {
                        item_tp.emplace_back(*ii);
                        (*count)++;
                    } else {
                        item_tp.emplace_back();
                    }
                }
            }

            // spdlog::error("resolve_time elapsed {:.3}", sw);

            //  only blank fraems
            auto bf = media::make_blank_frame(media_type);
            if (not *count) {
                for (auto i = 0; i < num_frames; i++)
                    (*result)[i] = bf;
                // spdlog::error("blank output elapsed {:.3}", sw);
                return *result;
            }

            auto rp = make_response_promise<media::AVFrameIDs>();

            auto start = 0;
            auto end   = 0;
            auto tps   = std::vector<FrameRate>();
            auto act   = caf::actor();

            for (auto i = 0; i < num_frames; i++) {
                auto item = item_tp[i];

                // dispatch on actor change
                if (not tps.empty() and (not item or item->first.actor() != act)) {
                    request(
                        act,
                        infinite,
                        media::get_media_pointer_atom_v,
                        media_type,
                        tps,
                        override_rate)
                        .then(
                            [=, s = start, e = end](const media::AVFrameIDs &mps) mutable {
                                for (auto ii = s; ii <= e; ii++) {
                                    (*result)[ii] = mps[ii - s];
                                    (*count)--;
                                    // spdlog::error("s {} e {} ii {} c {}", s, e, ii, *count);
                                    if (not *count) {
                                        rp.deliver(*result);
                                        // spdlog::error("get_media_pointers_atom elapsed
                                        // {:.3}", sw);
                                    }
                                }
                            },

                            [=, s = start, e = end](error &err) mutable {
                                for (auto ii = s; ii <= e; ii++) {
                                    (*result)[ii] = bf;
                                    (*count)--;
                                    // spdlog::error("s {} e {} ii {} c {}", s, e, ii, *count);
                                    if (not *count) {
                                        rp.deliver(*result);
                                        // spdlog::error("get_media_pointers_atom elapsed
                                        // {:.3}", sw);
                                    }
                                }
                            });

                    start = end = i;
                    tps.clear();
                    act = (item ? item->first.actor() : caf::actor());
                }

                if (not item) {
                    (*result)[i] = bf;
                } else {
                    if (tps.empty()) {
                        start = i;
                        act   = item->first.actor();
                    }
                    end = i;
                    tps.push_back(item->second);
                }
            }

            // catch all
            if (not tps.empty()) {
                request(
                    act,
                    infinite,
                    media::get_media_pointer_atom_v,
                    media_type,
                    tps,
                    override_rate)
                    .then(
                        [=, s = start, e = end](const media::AVFrameIDs &mps) mutable {
                            for (auto ii = s; ii <= e; ii++) {
                                (*result)[ii] = mps[ii - s];
                                (*count)--;
                                // spdlog::error("s {} e {} ii {} c {}", s, e, ii, *count);
                                if (not *count) {
                                    rp.deliver(*result);
                                    // spdlog::error("get_media_pointers_atom elapsed {:.3}",
                                    // sw);
                                }
                            }
                        },

                        [=, s = start, e = end](error &err) mutable {
                            for (auto ii = s; ii <= e; ii++) {
                                (*result)[ii] = bf;
                                (*count)--;
                                // spdlog::error("s {} e {} ii {} c {}", s, e, ii, *count);
                                if (not *count) {
                                    rp.deliver(*result);
                                    // spdlog::error("get_media_pointers_atom elapsed {:.3}",
                                    // sw);
                                }
                            }
                        });
            }

            // spdlog::error("get_media_pointers dispatched elapsed {:.3}", sw);
            return rp;
        },

        // [=](media::get_edit_list_atom, const Uuid &uuid) -> result<utility::EditList> {
        //     auto el = utility::EditList(utility::ClipList({utility::EditListSection(
        //         base_.uuid(),
        //         base_.item().trimmed_frame_duration(),
        //         utility::Timecode(
        //             base_.item().trimmed_frame_duration().frames(),
        //             base_.rate().to_fps()))}));
        //     return el;
        // },

        [=](utility::serialise_atom) -> result<JsonStore> {
            if (not actors_.empty()) {
                auto rp = make_response_promise<JsonStore>();

                fan_out_request<policy::select_all>(
                    map_value_to_vec(actors_), infinite, serialise_atom_v)
                    .then(
                        [=](std::vector<JsonStore> json) mutable {
                            JsonStore jsn;
                            jsn["base"]   = base_.serialise();
                            jsn["actors"] = {};
                            for (const auto &j : json)
                                jsn["actors"]
                                   [static_cast<std::string>(j["base"]["container"]["uuid"])] =
                                       j;
                            rp.deliver(jsn);
                        },
                        [=](error &err) mutable { rp.deliver(std::move(err)); });

                return rp;
            }
            JsonStore jsn;
            jsn["base"]   = base_.serialise();
            jsn["actors"] = {};

            return result<JsonStore>(jsn);
        },


        // ***********************************************************
        //
        // The following message handlers mirror PlaylistSelectionActor
        // and allow this actor to mimic this actor so that it works
        // with the playhead
        //
        // ***********************************************************

        [=](playhead::delete_selection_from_playlist_atom) {},

        [=](media_hook::gather_media_sources_atom) {},

        [=](playhead::evict_selection_from_cache_atom) -> media::MediaKeyVector {
            return media::MediaKeyVector();
        },

        [=](playhead::get_selection_atom) -> UuidList { return UuidList{base_.uuid()}; },

        [=](playhead::get_selection_atom, caf::actor requester) {
            anon_send(
                requester,
                utility::event_atom_v,
                playhead::selection_changed_atom_v,
                UuidList{base_.uuid()});
        },

        [=](playhead::select_next_media_atom, const int skip_by) {},

        [=](playlist::select_all_media_atom) {},

        [=](playlist::select_media_atom, const UuidList &media_uuids) {},

        [=](playlist::select_media_atom) {},

        [=](playlist::select_media_atom, utility::Uuid media_uuid) {},

        [=](playhead::get_selected_sources_atom) -> utility::UuidActorVector {
            return utility::UuidActorVector();
        },

        [=](session::get_playlist_atom) -> caf::actor {
            return caf::actor_cast<caf::actor>(playlist_);
        },

        [=](session::import_atom,
            const caf::uri &path,
            const std::string &data) -> result<bool> {
            auto rp = make_response_promise<bool>();
        // purge timeline.. ?
#ifdef BUILD_OTIO
            spawn(
                timeline_importer,
                rp,
                caf::actor_cast<caf::actor>(playlist_),
                UuidActor(base_.uuid(), actor_cast<caf::actor>(this)),
                path,
                data);
#else
            rp.deliver(
                make_error(xstudio_error::error, "OTIO IS NOT SUPPORTED IN THIS BUILD."));

#endif
            return rp;
        });
}

void TimelineActor::add_item(const utility::UuidActor &ua) {
    // join_event_group(this, ua.second);
    scoped_actor sys{system()};

    try {
        auto grp =
            request_receive<caf::actor>(*sys, ua.actor(), utility::get_event_group_atom_v);
        auto joined = request_receive<bool>(*sys, grp, broadcast::join_broadcast_atom_v, this);
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }

    monitor(ua.actor());
    actors_[ua.uuid()] = ua.actor();
}

void TimelineActor::add_media(
    caf::actor actor, const utility::Uuid &uuid, const utility::Uuid &before_uuid) {
    if (actor) {
        if (not base_.contains_media(uuid)) {
            base_.insert_media(uuid, before_uuid);
            media_actors_[uuid] = actor;
            monitor(actor);
        }
    } else {
        spdlog::warn("{} Invalid media actor", __PRETTY_FUNCTION__);
    }
}

void TimelineActor::add_media(
    const utility::UuidActor &ua,
    const utility::Uuid &before_uuid,
    caf::typed_response_promise<utility::UuidActor> rp) {

    try {

        caf::scoped_actor sys(system());

        // does the media already belong to parent playlist?
        auto actor = request_receive<caf::actor>(
            *sys,
            caf::actor_cast<caf::actor>(playlist_),
            playlist::get_media_atom_v,
            ua.uuid(),
            true);

        if (!actor) {

            // no, parent playlist doesn't have this media so we must add it first
            actor = request_receive<UuidActor>(
                        *sys,
                        caf::actor_cast<caf::actor>(playlist_),
                        playlist::add_media_atom_v,
                        ua,
                        before_uuid)
                        .actor();
        }

        add_media(actor, ua.uuid(), before_uuid);
        send(
            event_group_,
            utility::event_atom_v,
            playlist::add_media_atom_v,
            UuidActorVector({ua}));
        base_.send_changed(event_group_, this);
        send(event_group_, utility::event_atom_v, change_atom_v);

        // send(change_event_group_, utility::event_atom_v, utility::change_atom_v);

        rp.deliver(ua);

    } catch (const std::exception &err) {
        rp.deliver(make_error(xstudio_error::error, err.what()));
    }
}

bool TimelineActor::remove_media(caf::actor actor, const utility::Uuid &uuid) {
    bool result = false;

    if (base_.remove_media(uuid)) {
        demonitor(actor);
        media_actors_.erase(uuid);
        result = true;
    }

    return result;
}

void TimelineActor::on_exit() {
    for (const auto &i : actors_)
        send_exit(i.second, caf::exit_reason::user_shutdown);
}

void TimelineActor::deliver_media_pointer(
    const int logical_frame,
    const media::MediaType media_type,
    caf::typed_response_promise<media::AVFrameID> rp) {

    std::vector<caf::actor> actors;
    for (const auto &i : base_.media())
        actors.push_back(actors_[i]);

    fan_out_request<policy::select_all>(actors, infinite, media::media_reference_atom_v, Uuid())
        .then(
            [=](std::vector<std::pair<Uuid, MediaReference>> refs) mutable {
                // re-order vector based on playlist order
                std::vector<std::pair<Uuid, MediaReference>> ordered_refs;
                for (const auto &i : base_.media()) {
                    for (const auto &ii : refs) {
                        const auto &[uuid, ref] = ii;
                        if (uuid == i) {
                            ordered_refs.push_back(ii);
                            break;
                        }
                    }
                }

                // step though list, and find the relevant ref..
                std::pair<Uuid, MediaReference> m;
                int frames             = 0;
                bool exceeded_duration = true;

                for (auto it = std::begin(ordered_refs); it != std::end(ordered_refs); ++it) {
                    if ((logical_frame - frames) < it->second.duration().frames()) {
                        m                 = *it;
                        exceeded_duration = false;
                        break;
                    }
                    frames += it->second.duration().frames();
                }

                try {
                    if (exceeded_duration)
                        throw std::runtime_error("No frames left");
                    // send request media atom..
                    request(
                        actors_[m.first],
                        infinite,
                        media::get_media_pointer_atom_v,
                        media_type,
                        logical_frame - frames)
                        .then(
                            [=](const media::AVFrameID &mp) mutable { rp.deliver(mp); },
                            [=](error &err) mutable { rp.deliver(std::move(err)); });

                } catch (const std::exception &e) {
                    rp.deliver(make_error(xstudio_error::error, e.what()));
                }
            },
            [=](error &err) mutable { rp.deliver(std::move(err)); });
}


void TimelineActor::sort_by_media_display_info(
    const int info_set_idx, const int info_item_idx, const bool ascending) {

    using SourceAndUuid = std::pair<std::string, utility::Uuid>;
    auto media_names_vs_uuids =
        std::make_shared<std::vector<std::pair<std::string, utility::Uuid>>>();

    for (const auto &i : base_.media()) {

        // Pro tip: because i is a reference, it's the reference that is captured in our lambda
        // below and therefore it is 'unstable' so we make a copy here and use that in the
        // lambda as this is object-copied in the capture instead.
        UuidActor media_actor(i, media_actors_[i]);

        request(media_actor.actor(), infinite, media::media_reference_atom_v, utility::Uuid())
            .await(

                [=](const std::pair<Uuid, MediaReference> &m_ref) mutable {
                    std::string path = uri_to_posix_path(m_ref.second.uri());
                    path             = std::string(path, path.rfind("/") + 1);
                    path             = to_lower(path);

                    (*media_names_vs_uuids).push_back(std::make_pair(path, media_actor.uuid()));

                    if (media_names_vs_uuids->size() == base_.media().size()) {

                        std::sort(
                            media_names_vs_uuids->begin(),
                            media_names_vs_uuids->end(),
                            [](const SourceAndUuid &a, const SourceAndUuid &b) -> bool {
                                return a.first < b.first;
                            });

                        utility::UuidList ordered_uuids;
                        for (const auto &p : (*media_names_vs_uuids)) {
                            ordered_uuids.push_back(p.second);
                        }

                        anon_send(
                            caf::actor_cast<caf::actor>(this),
                            playlist::move_media_atom_v,
                            ordered_uuids,
                            utility::Uuid());
                    }
                },
                [=](error &err) mutable {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, to_string(err));
                });
    }
}

void TimelineActor::insert_items(
    const int index,
    const UuidActorVector &uav,
    caf::typed_response_promise<utility::JsonStore> rp) {
    // validate items can be inserted.
    fan_out_request<policy::select_all>(vector_to_caf_actor_vector(uav), infinite, item_atom_v)
        .then(
            [=](std::vector<Item> items) mutable {
                // items are valid for insertion ?
                for (const auto &i : items) {
                    if (not base_.item().valid_child(i))
                        return rp.deliver(
                            make_error(xstudio_error::error, "Invalid child type"));
                }

                // take ownership
                for (const auto &ua : uav)
                    add_item(ua);

                // find insertion point..
                auto it = std::next(base_.item().begin(), index);

                // insert items..
                // our list will be out of order..
                auto changes = JsonStore(R"([])"_json);
                for (const auto &ua : uav) {
                    // find item..
                    auto found = false;
                    for (const auto &i : items) {
                        if (ua.uuid() == i.uuid()) {
                            auto tmp = base_.item().insert(it, i);
                            changes.insert(changes.end(), tmp.begin(), tmp.end());
                            found = true;
                            break;
                        }
                    }

                    if (not found) {
                        spdlog::error("item not found for insertion");
                    }
                }

                // add changes to stack
                auto more = base_.item().refresh();

                if (not more.is_null())
                    changes.insert(changes.begin(), more.begin(), more.end());

                send(event_group_, event_atom_v, item_atom_v, changes, false);
                anon_send(history_, history::log_atom_v, __sysclock_now(), changes);
                send(this, utility::event_atom_v, change_atom_v);

                rp.deliver(changes);
            },
            [=](const caf::error &err) mutable { rp.deliver(err); });
}

void TimelineActor::remove_items(
    const int index,
    const int count,
    caf::typed_response_promise<std::pair<utility::JsonStore, std::vector<timeline::Item>>>
        rp) {

    std::vector<Item> items;
    JsonStore changes(R"([])"_json);

    if (index < 0 or index + count - 1 >= static_cast<int>(base_.item().size()))
        rp.deliver(make_error(xstudio_error::error, "Invalid index / count"));
    else {
        scoped_actor sys{system()};

        for (int i = index + count - 1; i >= index; i--) {
            auto it = std::next(base_.item().begin(), i);
            if (it != base_.item().end()) {
                auto item = *it;
                demonitor(item.actor());
                actors_.erase(item.uuid());
                auto blind = request_receive<JsonStore>(*sys, item.actor(), serialise_atom_v);

                auto tmp = base_.item().erase(it, blind);
                changes.insert(changes.end(), tmp.begin(), tmp.end());
                items.push_back(item);
            }
        }

        auto more = base_.item().refresh();
        if (not more.is_null())
            changes.insert(changes.begin(), more.begin(), more.end());

        // why was this commented out ?
        // send(event_group_, event_atom_v, item_atom_v, changes, false);

        anon_send(history_, history::log_atom_v, __sysclock_now(), changes);

        send(this, utility::event_atom_v, change_atom_v);

        rp.deliver(std::make_pair(changes, items));
    }
}

void TimelineActor::erase_items(
    const int index, const int count, caf::typed_response_promise<JsonStore> rp) {

    request(
        caf::actor_cast<caf::actor>(this), infinite, remove_item_atom_v, index, count, false)
        .then(
            [=](const std::pair<JsonStore, std::vector<Item>> &hist_item) mutable {
                for (const auto &i : hist_item.second)
                    send_exit(i.actor(), caf::exit_reason::user_shutdown);
                rp.deliver(hist_item.first);
            },
            [=](error &err) mutable { rp.deliver(std::move(err)); });
}

// create new track from bake list
void TimelineActor::bake(
    caf::typed_response_promise<utility::UuidActor> rp, const utility::UuidSet &uuids) {
    // audio or video ?
    //  bake for range of timeline
    if (uuids.empty())
        return rp.deliver(make_error(xstudio_error::error, "Empty uuid list"));

    auto items = std::vector<std::optional<ResolvedItem>>();
    auto range = base_.item().trimmed_range();

    auto first    = range.start();
    auto duration = range.duration();
    auto last     = range.start() + range.duration();
    auto rate     = range.rate();

    auto mtype = media::MediaType::MT_IMAGE;

    // determine if this is audio or image
    // find track container of uuid.

    auto utrack = find_track_from_item(base_.item().children(), *(uuids.begin()));
    if (utrack) {
        // spdlog::warn("{}", (*utrack)->name());
        if ((*utrack)->item_type() == IT_AUDIO_TRACK)
            mtype = media::MediaType::MT_AUDIO;
    }

    // spdlog::warn("{} {} {}", range.frame_start().frames(), range.frame_duration().frames(),
    // to_string(range.rate())); spdlog::warn("{} {} {} {}", first.to_seconds(),
    // duration.to_seconds(), last.to_seconds(), to_string(range.rate()));

    for (auto i = first; i <= last; i += range.rate()) {
        auto r = base_.item().resolve_time(i, mtype, uuids, true);
        // if(r)
        //     spdlog::warn("frame {} {} {}", i.to_seconds(), std::get<1>(*r).to_seconds(),
        //     std::get<0>(*r).name());
        // else
        //     spdlog::warn("frame {}", i.to_seconds());
        items.emplace_back(r);
    }

    // collapse into sequence of clips and gaps.
    auto track = Track("Flattened Tracks", mtype);

    for (const auto &i : items) {
        if (track.children().empty() or                                  // empty track
            (track.children().back().item_type() != IT_GAP and not i) or // change to gap
            (track.children().back().item_type() == IT_GAP and i) or     // change from gap
            (track.children().back().item_type() != IT_GAP and
             track.children().back().uuid() !=
                 std::get<0>(*i).uuid()) // change to different clip
        ) {
            if (not i) {
                track.children().emplace_back(
                    Gap("Gap", utility::FrameRateDuration(1, rate)).item());
            } else {
                // replace item with copy of i
                track.children().emplace_back(std::get<0>(*i));
                // track.children().back().set_uuid(utility::Uuid::generate());
                track.children().back().set_actor_addr(caf::actor_addr());
                track.children().back().set_active_range(FrameRange(
                    FrameRateDuration(
                        std::get<1>(*i).to_flicks(), track.children().back().rate()),
                    FrameRateDuration(1, track.children().back().rate())));
            }
        } else {
            if (i) {
                auto trange = track.children().back().trimmed_range();
                trange.set_duration(trange.duration() + trange.rate());
                track.children().back().set_active_range(trange);
            } else {
                auto trange = track.children().back().trimmed_range();
                trange.set_duration(trange.duration() + trange.rate());
                track.children().back().set_active_range(trange);
            }
        }
    }
    // trim trailing gap..
    if (not track.children().empty() and track.children().back().item_type() == IT_GAP)
        track.children().pop_back();

    // reset uuids
    for (auto &i : track.item().children()) {
        i.set_uuid(utility::Uuid::generate());
    }

    track.item().refresh();

    // for(const auto &i: track.item().children()) {
    //     spdlog::warn("{} {} {}", i.name(), i.trimmed_frame_start().frames(),
    //     i.trimmed_frame_duration().frames());
    // }

    // create actors
    auto track_uuid  = track.item().uuid();
    auto track_actor = spawn<TrackActor>(track.item());

    rp.deliver(UuidActor(track_uuid, track_actor));
}
