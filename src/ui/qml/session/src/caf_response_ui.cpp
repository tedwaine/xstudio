// SPDX-License-Identifier: Apache-2.0

#include "xstudio/media/media.hpp"
#include "xstudio/session/session_actor.hpp"
#include "xstudio/timeline/item.hpp"
#include "xstudio/ui/qml/caf_response_ui.hpp"
#include "xstudio/ui/qml/job_control_ui.hpp"
#include "xstudio/ui/qml/json_tree_model_ui.hpp"
#include "xstudio/ui/qml/session_model_ui.hpp"

#include <nlohmann/json.hpp>

CAF_PUSH_WARNINGS
#include <QThreadPool>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QSignalSpy>
CAF_POP_WARNINGS

using namespace caf;
using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::ui::qml;

class CafRequest : public ControllableJob<QMap<int, QString>> {
  public:
    CafRequest(const nlohmann::json json, const int role, const std::string role_name)
        : ControllableJob(),
          json_(std::move(json)),
          role_(role),
          role_name_(std::move(role_name)) {}

    QMap<int, QString> run(JobControl &cjc) override {
        QMap<int, QString> result;

        try {
            if (not cjc.shouldRun())
                throw std::runtime_error("Cancelled");

            const auto type = json_.at("type").get<std::string>();

            caf::actor_system &system_ = CafSystemObject::get_actor_system();
            scoped_actor sys{system_};

            switch (role_) {

            case SessionModel::Roles::thumbnailURLRole:
                if (type == "MediaSource") {
                    // get current source/stream..

                    QString thumburl("qrc:///feather_icons/film.svg");
                    try {
                        // get mediasource stream detail.
                        auto mr = request_receive<utility::MediaReference>(
                            *sys,
                            actorFromString(system_, json_.at("actor")),
                            media::media_reference_atom_v);
                        if (mr.frame_count() != 0) {
                            auto middle_frame = (mr.frame_count() - 1) / 2;
                            thumburl          = getThumbnailURL(
                                system_,
                                actorFromString(system_, json_.at("actor")),
                                middle_frame,
                                true);
                            result[SessionModel::Roles::thumbnailURLRole] =
                                QStringFromStd(json(StdFromQString(thumburl)).dump());
                        } else {
                            // ignore leave null..
                            // and leave for a retry..?
                            result[SessionModel::Roles::thumbnailURLRole] =
                                QStringFromStd(json("RETRY").dump());
                        }
                    } catch (const std::exception &err) {
                        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    }
                }
                break;

            case SessionModel::Roles::rateFPSRole:
                if (type == "MediaSource") {
                    auto rr = request_receive<MediaReference>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        media::media_reference_atom_v);

                    result[SessionModel::Roles::timecodeAsFramesRole] =
                        QStringFromStd(json(rr.timecode().total_frames()).dump());
                    result[SessionModel::Roles::pathRole] =
                        QStringFromStd(json(to_string(rr.uri())).dump());
                    result[SessionModel::Roles::pathShakeRole] = QStringFromStd(
                        json(to_string(rr.uri(MediaReference::FramePadFormat::FPF_SHAKE)))
                            .dump());
                    result[SessionModel::Roles::rateFPSRole] =
                        QStringFromStd(json(rr.rate()).dump());
                } else if (type == "Session") {
                    auto value = request_receive<FrameRate>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        session::media_rate_atom_v);
                    result[SessionModel::Roles::rateFPSRole] =
                        QStringFromStd(json(value).dump());
                }
                break;

            case SessionModel::Roles::bookmarkUuidsRole:
                if (type == "Media") {

                    auto uids = request_receive<utility::UuidList>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        bookmark::get_bookmarks_atom_v);
                    auto j = nlohmann::json::array();
                    for (const auto &uuid : uids) {
                        j.push_back(uuid);
                    }
                    result[SessionModel::Roles::bookmarkUuidsRole] = QStringFromStd(j.dump());
                }
                break;
            case SessionModel::Roles::bitDepthRole:
            case SessionModel::Roles::resolutionRole:
            case SessionModel::Roles::formatRole:
            case SessionModel::Roles::pixelAspectRole:
                if (type == "MediaSource") {

                    std::string path;
                    switch (role_) {
                    case SessionModel::Roles::bitDepthRole:
                        path = "regex:/metadata/media/.*standard_fields/bit_depth";
                        break;
                    case SessionModel::Roles::formatRole:
                        path = "regex:/metadata/media/.*standard_fields/format";
                        break;
                    case SessionModel::Roles::resolutionRole:
                        path = "regex:/metadata/media/.*standard_fields/resolution";
                        break;
                    case SessionModel::Roles::pixelAspectRole:
                        path = "regex:/metadata/media/.*standard_fields/pixel_aspect";
                        break;
                    default:
                        break;
                    }

                    auto data = request_receive<JsonStore>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        json_store::get_json_atom_v,
                        path);

                    result[role_] = QStringFromStd(data.dump());
                }
                break;

            case SessionModel::Roles::pathRole:
                if (type == "Session") {
                    auto pt = request_receive<std::pair<caf::uri, fs::file_time_type>>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        session::path_atom_v);

                    result[SessionModel::Roles::pathRole] =
                        QStringFromStd(json(to_string(pt.first)).dump());
                    result[SessionModel::Roles::mtimeRole] =
                        QStringFromStd(json(pt.second).dump());
                } else if (type == "MediaSource") {
                    auto rr = request_receive<MediaReference>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        media::media_reference_atom_v);

                    result[SessionModel::Roles::timecodeAsFramesRole] =
                        QStringFromStd(json(rr.timecode().total_frames()).dump());
                    result[SessionModel::Roles::pathRole] =
                        QStringFromStd(json(to_string(rr.uri())).dump());
                    result[SessionModel::Roles::pathShakeRole] = QStringFromStd(
                        json(to_string(rr.uri(MediaReference::FramePadFormat::FPF_SHAKE)))
                            .dump());
                    result[SessionModel::Roles::rateFPSRole] =
                        QStringFromStd(json(rr.rate()).dump());
                }
                break;
            case SessionModel::Roles::pathShakeRole:
                if (type == "MediaSource") {
                    auto rr = request_receive<MediaReference>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        media::media_reference_atom_v);

                    result[SessionModel::Roles::timecodeAsFramesRole] =
                        QStringFromStd(json(rr.timecode().total_frames()).dump());
                    result[SessionModel::Roles::pathRole] =
                        QStringFromStd(json(to_string(rr.uri())).dump());
                    result[SessionModel::Roles::pathShakeRole] = QStringFromStd(
                        json(to_string(rr.uri(MediaReference::FramePadFormat::FPF_SHAKE)))
                            .dump());
                    result[SessionModel::Roles::rateFPSRole] =
                        QStringFromStd(json(rr.rate()).dump());
                }
                break;
            case SessionModel::Roles::nameRole:
            case SessionModel::Roles::actorUuidRole:
            case SessionModel::Roles::groupActorRole:
            case SessionModel::Roles::typeRole:
                if (type == "Session" or type == "Playlist" or type == "Subset" or
                    type == "Timeline" or type == "Media" or type == "PlayheadSelection" or
                    type == "Playhead" or type == "Aux Playhead") {

                    auto actor = caf::actor();

                    if (json_.count("actor") and not json_.at("actor").is_null()) {
                        actor = actorFromString(system_, json_.at("actor"));
                    } else if (
                        not json_.at("actor_owner").is_null() and type == "PlayheadSelection") {
                        // get selection actor from owner
                        actor = request_receive<caf::actor>(
                            *sys,
                            actorFromString(system_, json_.at("actor_owner")),
                            playlist::selection_actor_atom_v);

                        result[SessionModel::Roles::actorRole] =
                            QStringFromStd(json(actorToString(system_, actor)).dump());
                    } else if (not json_.at("actor_owner").is_null() and type == "Playhead") {
                        // get selection actor from owner

                        auto playhead = request_receive<utility::UuidActor>(
                            *sys,
                            actorFromString(system_, json_.at("actor_owner")),
                            playlist::get_playhead_atom_v);

                        result[SessionModel::Roles::actorRole] = QStringFromStd(
                            json(actorToString(system_, playhead.actor())).dump());

                        result[SessionModel::Roles::actorUuidRole] =
                            QStringFromStd(json(to_string(playhead.uuid())).dump());

                    } else if (
                        not json_.at("actor_owner").is_null() and type == "Aux Playhead") {
                        // get selection actor from owner

                        auto playhead = request_receive<utility::UuidActor>(
                            *sys,
                            actorFromString(system_, json_.at("actor_owner")),
                            playlist::get_playhead_atom_v,
                            1); // 1 is index of Aux playhead for timelines

                        result[SessionModel::Roles::actorRole] = QStringFromStd(
                            json(actorToString(system_, playhead.actor())).dump());

                        result[SessionModel::Roles::actorUuidRole] =
                            QStringFromStd(json(to_string(playhead.uuid())).dump());
                    }

                    if (actor) {
                        // get detail.
                        auto detail = request_receive<ContainerDetail>(
                            *sys, actor, utility::detail_atom_v);

                        result[SessionModel::Roles::groupActorRole] =
                            QStringFromStd(json(actorToString(system_, detail.group_)).dump());
                        result[SessionModel::Roles::nameRole] =
                            QStringFromStd(json(detail.name_).dump());
                        result[SessionModel::Roles::actorUuidRole] =
                            QStringFromStd(json(detail.uuid_).dump());
                        result[SessionModel::Roles::typeRole] =
                            QStringFromStd(json(detail.type_).dump());
                    }
                }
                break;

            case SessionModel::expandedRole:
                if (type == "Playlist") {
                    auto target = actorFromString(system_, json_.at("actor"));
                    if (target) {
                        try {
                            auto answer =
                                request_receive<bool>(*sys, target, playlist::expanded_atom_v);
                            result[SessionModel::Roles::expandedRole] =
                                QStringFromStd(json(answer).dump());
                        } catch (std::exception &e) {
                            // silence if no sources..
                        }
                    }
                }
                break;

            case SessionModel::mediaDisplayInfoRole:
                if (type == "Media") {
                    auto target = actorFromString(system_, json_.at("actor"));

                    if (target) {
                        try {
                            auto answer = request_receive<utility::JsonStore>(
                                *sys, target, media::media_display_info_atom_v);
                            result[SessionModel::Roles::mediaDisplayInfoRole] =
                                QStringFromStd(answer.dump());
                        } catch (std::exception &e) {
                            // silence if no sources..
                        }
                    }
                }
                break;

            case SessionModel::timecodeAsFramesRole:
                if (type == "MediaSource") {
                    auto rr = request_receive<MediaReference>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        media::media_reference_atom_v);

                    result[SessionModel::Roles::timecodeAsFramesRole] =
                        QStringFromStd(json(rr.timecode().total_frames()).dump());
                    result[SessionModel::Roles::pathRole] =
                        QStringFromStd(json(to_string(rr.uri())).dump());
                    result[SessionModel::Roles::pathShakeRole] = QStringFromStd(
                        json(to_string(rr.uri(MediaReference::FramePadFormat::FPF_SHAKE)))
                            .dump());
                    result[SessionModel::Roles::rateFPSRole] =
                        QStringFromStd(json(rr.rate()).dump());
                }
                break;

            case SessionModel::Roles::mediaStatusRole:
                if (type == "Media" or type == "MediaSource") {
                    auto target = actorFromString(system_, json_.at("actor"));

                    if (target) {
                        try {
                            auto answer = request_receive<media::MediaStatus>(
                                *sys, target, media::media_status_atom_v);
                            result[SessionModel::Roles::mediaStatusRole] =
                                QStringFromStd(json(answer).dump());
                        } catch (...) {
                            // silence if no sources..
                        }
                    }
                }
                break;

            case SessionModel::Roles::imageActorUuidRole:
                if (type == "Media") {
                    auto target = actorFromString(system_, json_.at("actor"));
                    if (target) {
                        auto answer = request_receive<UuidActor>(
                            *sys, target, media::current_media_source_atom_v, media::MT_IMAGE);

                        result[SessionModel::Roles::imageActorUuidRole] =
                            QStringFromStd(json(answer.uuid()).dump());
                    }
                } else if (type == "MediaSource") {
                    auto answer = request_receive<UuidActor>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        media::current_media_stream_atom_v,
                        media::MT_IMAGE);

                    result[SessionModel::Roles::imageActorUuidRole] =
                        QStringFromStd(json(answer.uuid()).dump());
                }
                break;

            case SessionModel::Roles::flagColourRole:
            case SessionModel::Roles::flagTextRole:
                if (type == "Media") {
                    auto target = actorFromString(system_, json_.at("actor"));
                    if (target) {
                        auto [flag, text] =
                            request_receive<std::tuple<std::string, std::string>>(
                                *sys, target, playlist::reflag_container_atom_v);

                        result[SessionModel::Roles::flagColourRole] =
                            QStringFromStd(json(flag).dump());
                        result[SessionModel::Roles::flagTextRole] =
                            QStringFromStd(json(text).dump());
                    }
                }
                break;
            case SessionModel::Roles::selectionRole: {
                if (type == "PlayheadSelection" && !json_.at("actor").is_null()) {
                    auto target = actorFromString(system_, json_.at("actor"));
                    if (target) {

                        auto selection = request_receive<utility::UuidList>(
                            *sys, target, playhead::get_selection_atom_v);

                        auto j = nlohmann::json::array();
                        for (const auto &uuid : selection) {
                            j.push_back(uuid);
                        }
                        result[SessionModel::Roles::selectionRole] = QStringFromStd(j.dump());
                    }
                }
            } break;
            case SessionModel::Roles::audioActorUuidRole:
                if (type == "Media") {
                    try {
                        auto answer = request_receive<UuidActor>(
                            *sys,
                            actorFromString(system_, json_.at("actor")),
                            media::current_media_source_atom_v,
                            media::MT_AUDIO);

                        result[SessionModel::Roles::audioActorUuidRole] =
                            QStringFromStd(json(answer.uuid()).dump());
                    } catch (...) {
                        // no audio source..
                        result[SessionModel::Roles::audioActorUuidRole] =
                            QStringFromStd(json(Uuid()).dump());
                    }
                } else if (type == "MediaSource") {
                    try {
                        auto answer = request_receive<UuidActor>(
                            *sys,
                            actorFromString(system_, json_.at("actor")),
                            media::current_media_stream_atom_v,
                            media::MT_AUDIO);

                        result[SessionModel::Roles::audioActorUuidRole] =
                            QStringFromStd(json(answer.uuid()).dump());
                    } catch (...) {
                        // no audio source..
                        result[SessionModel::Roles::audioActorUuidRole] =
                            QStringFromStd(json(Uuid()).dump());
                    }
                }
                break;

            case JSONTreeModel::Roles::childrenRole: {
                if (type == "Session") {
                    auto session    = actorFromString(system_, json_.at("actor"));
                    auto containers = request_receive<utility::PlaylistTree>(
                        *sys, session, playlist::get_container_atom_v);
                    auto actors = request_receive<std::vector<UuidActor>>(
                        *sys, session, session::get_playlists_atom_v);

                    // session tree contains...

                    result[JSONTreeModel::Roles::childrenRole] =
                        QStringFromStd(SessionModel::sessionTreeToJson(
                                           containers, system_, uuidactor_vect_to_map(actors))
                                           .at("children")
                                           .dump());

                } else if (type == "Media List") {
                    // spdlog::error(
                    //     "get Media List children {} {}",
                    //     json_.dump(2),
                    //     to_string(actorFromString(system_, json_.at("actor_owner")))
                    //     );

                    auto detail = request_receive<std::vector<ContainerDetail>>(
                        *sys,
                        actorFromString(system_, json_.at("actor_owner")),
                        playlist::get_media_atom_v,
                        true);

                    auto jsn = R"([])"_json;
                    for (const auto &i : detail)
                        jsn.emplace_back(SessionModel::containerDetailToJson(i, system_));

                    result[JSONTreeModel::Roles::childrenRole] = QStringFromStd(jsn.dump());
                } else if (type == "Clip") {
                    auto target = actorFromString(system_, json_.at("actor"));

                    auto mua = request_receive<utility::UuidActor>(
                        *sys, target, playlist::get_media_atom_v);
                    auto detail = request_receive<ContainerDetail>(
                        *sys, mua.actor(), utility::detail_atom_v);

                    auto jsn = R"([])"_json;
                    jsn.emplace_back(SessionModel::containerDetailToJson(detail, system_));

                    result[JSONTreeModel::Roles::childrenRole] = QStringFromStd(jsn.dump());
                } else if (type == "Container List") {
                    // // only happens from playlist.
                    auto containers = request_receive<utility::PlaylistTree>(
                        *sys,
                        actorFromString(system_, json_.at("actor_owner")),
                        playlist::get_container_atom_v);
                    auto actors = request_receive<std::vector<UuidActor>>(
                        *sys,
                        actorFromString(system_, json_.at("actor_owner")),
                        playlist::get_container_atom_v,
                        true);

                    result[JSONTreeModel::Roles::childrenRole] =
                        QStringFromStd(SessionModel::playlistTreeToJson(
                                           containers, system_, uuidactor_vect_to_map(actors))
                                           .at("children")
                                           .dump());

                } else if (type == "Media") {
                    auto target = actorFromString(system_, json_.at("actor"));
                    if (target) {
                        auto detail = request_receive<std::vector<ContainerDetail>>(
                            *sys, target, utility::detail_atom_v, true);
                        auto jsn = R"([])"_json;
                        for (const auto &i : detail)
                            jsn.emplace_back(SessionModel::containerDetailToJson(i, system_));
                        result[JSONTreeModel::Roles::childrenRole] = QStringFromStd(jsn.dump());
                    }
                } else if (type == "TimelineItem") {
                    auto owner = actorFromString(system_, json_.at("actor_owner"));
                    auto item =
                        request_receive<timeline::Item>(*sys, owner, timeline::item_atom_v);

                    // we want our own instance of the item..
                    result[JSONTreeModel::Roles::JSONTextRole] =
                        QStringFromStd(item.serialise().dump());

                    // spdlog::warn("{}", jsn.dump(2));
                    // auto jsn = SessionModel::timelineItemToJson(item, system_);
                    // result[SessionModel::Roles::rateRole] =
                    // QStringFromStd(jsn.at("rate").dump());
                    // result[SessionModel::Roles::trimmedRangeRole] =
                    // QStringFromStd(jsn.at("trimmed_range").dump());
                    // result[SessionModel::Roles::activeRangeRole] =
                    // QStringFromStd(jsn.at("active_range").dump());
                    // result[SessionModel::Roles::availableRangeRole] =
                    // QStringFromStd(jsn.at("available_range").dump());
                    // result[SessionModel::Roles::enabledRole] =
                    // QStringFromStd(jsn.at("enabled").dump());
                    // result[SessionModel::Roles::transparentRole] =
                    // QStringFromStd(jsn.at("transparent").dump());
                    // result[SessionModel::Roles::uuidRole] =
                    // QStringFromStd(jsn.at("uuid").dump());
                    // result[SessionModel::Roles::actorRole] =
                    // QStringFromStd(jsn.at("actor").dump());
                    // // result[SessionModel::Roles::typeRole] =
                    // QStringFromStd(jsn.at("type").dump());

                    // result[JSONTreeModel::Roles::childrenRole] =
                    // QStringFromStd(jsn.at("children").dump()); we can also update other
                    // fields..

                } else if (type == "MediaSource") {

                    auto idetail = request_receive<std::vector<ContainerDetail>>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        utility::detail_atom_v,
                        media::MT_IMAGE);
                    auto adetail = request_receive<std::vector<ContainerDetail>>(
                        *sys,
                        actorFromString(system_, json_.at("actor")),
                        utility::detail_atom_v,
                        media::MT_AUDIO);

                    auto jsn = R"([{},{}])"_json;

                    jsn[0] = SessionModel::createEntry(
                        R"({"type": "Image Stream", "children": [], "actor_owner": null})"_json);
                    jsn[0]["actor_owner"] = json_.at("actor");
                    jsn[1]                = SessionModel::createEntry(
                        R"({"type": "Audio Stream", "children": [], "actor_owner": null})"_json);
                    jsn[1]["actor_owner"] = json_.at("actor");

                    for (const auto &i : idetail)
                        jsn[0]["children"].emplace_back(
                            SessionModel::containerDetailToJson(i, system_));
                    for (const auto &i : adetail)
                        jsn[1]["children"].emplace_back(
                            SessionModel::containerDetailToJson(i, system_));
                    result[JSONTreeModel::Roles::childrenRole] = QStringFromStd(jsn.dump());
                } else if (type == "PlayheadSelection") {
                    auto actor = caf::actor();
                    // spdlog::warn("request children PlayheadSelection {}", json_.dump(2));

                    if (not json_.at("actor").is_null()) {
                        actor = actorFromString(system_, json_.at("actor"));
                    } else if (not json_.at("actor_owner").is_null()) {
                        // spdlog::warn("PlayheadSelection {}",
                        // json_.at("actor_owner").get<std::string>()); get selection actor from
                        // owner
                        actor = request_receive<caf::actor>(
                            *sys,
                            actorFromString(system_, json_.at("actor_owner")),
                            playlist::selection_actor_atom_v);
                        // get detail.
                        auto detail = request_receive<ContainerDetail>(
                            *sys, actor, utility::detail_atom_v);

                        result[SessionModel::Roles::groupActorRole] =
                            QStringFromStd(json(actorToString(system_, detail.group_)).dump());
                        result[SessionModel::Roles::nameRole] =
                            QStringFromStd(json(detail.name_).dump());
                        result[SessionModel::Roles::actorRole] =
                            QStringFromStd(json(actorToString(system_, actor)).dump());
                        result[SessionModel::Roles::actorUuidRole] =
                            QStringFromStd(json(detail.uuid_).dump());
                        result[SessionModel::Roles::typeRole] =
                            QStringFromStd(json(detail.type_).dump());

                        // spdlog::warn("{}", detail.name_);
                    }

                    if (actor) {
                        auto selection = request_receive<UuidList>(
                            *sys, actor, playhead::get_selection_atom_v);
                        auto jsn       = R"([])"_json;
                        auto actor_str = actorToString(system_, actor);
                        for (const auto &i : selection) {
                            auto sel = SessionModel::createEntry(
                                R"({"type": "Uuid", "uuid": null, "actor_owner": null})"_json);
                            sel.erase("children");
                            sel["uuid"]        = i;
                            sel["actor_owner"] = actor_str;
                            jsn.emplace_back(sel);
                        }
                        // spdlog::warn("{}", jsn.dump(2));
                        result[JSONTreeModel::Roles::childrenRole] = QStringFromStd(jsn.dump());
                    }


                } else if (type == "Playhead" || type == "Aux Playhead") {
                    // Playhead has no children
                } else {
                    spdlog::warn("CafRequest unhandled ChildrenRole type {}", type);
                }}
                break;
            default: {
            } break;
            }

        } catch (const std::exception &err) {

            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }

        return result;
    }

  private:
    const nlohmann::json json_;
    const int role_;
    const std::string role_name_;
};

static int ct = 0;

CafResponse::CafResponse(
    const QVariant search_value,
    const int search_role,
    const QPersistentModelIndex search_hint,
    const nlohmann::json &data,
    const int role,
    const std::string &role_name,
    QThreadPool *pool)
    : search_value_(std::move(search_value)),
      search_role_(search_role),
      search_hint_(std::move(search_hint)),
      role_(role) {

    // create a future..
    connect(
        &watcher_,
        &QFutureWatcher<QMap<int, QString>>::finished,
        this,
        &CafResponse::handleFinished);

    connect(
        &watcher_,
        &QFutureWatcher<QMap<int, QString>>::started,
        this,
        &CafResponse::handleStarted);

    try {
        QFuture<QMap<int, QString>> future =
            JobExecutor::run(new CafRequest(data, role, role_name), pool);

        watcher_.setFuture(future);
    } catch (...) {
        deleteLater();
    }
}

CafResponse::~CafResponse() {}

void CafResponse::handleFinished() {
    emit finished(search_value_, search_role_, role_);

    if (watcher_.future().resultCount()) {
        auto result = watcher_.result();

        QMap<int, QString>::const_iterator i = result.constBegin();
        while (i != result.constEnd()) {
            emit received(search_value_, search_role_, search_hint_, i.key(), i.value());
            ++i;
        }

        deleteLater();
    }
}

void CafResponse::handleStarted() { emit started(search_value_, search_role_, role_); }
