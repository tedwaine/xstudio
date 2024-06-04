// SPDX-License-Identifier: Apache-2.0

#include "xstudio/conform/conformer.hpp"
#include "xstudio/ui/qml/conform_ui.hpp"
#include "xstudio/ui/qml/json_tree_model_ui.hpp"
#include "xstudio/ui/qml/session_model_ui.hpp"

CAF_PUSH_WARNINGS
CAF_POP_WARNINGS

using namespace caf;
using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::ui::qml;

ConformEngineUI::ConformEngineUI(QObject *parent) : super(parent) {
    init(CafSystemObject::get_actor_system());

    setRoleNames(std::vector<std::string>({"nameRole"}));
}


void ConformEngineUI::init(caf::actor_system &_system) {
    super::init(_system);

    self()->set_default_handler(caf::drop);

    // join conform engine events.
    try {
        scoped_actor sys{system()};
        utility::print_on_create(as_actor(), "ConformEngineUI");
        auto conform_manager = system().registry().template get<caf::actor>(conform_registry);

        try {
            auto uuids =
                request_receive<UuidVector>(*sys, conform_manager, json_store::sync_atom_v);
            conform_uuid_ = uuids[0];

            // get system presets
            auto data = request_receive<JsonStore>(
                *sys, conform_manager, json_store::sync_atom_v, conform_uuid_);
            setModelData(data);

            // join events.
            if (conform_events_) {
                try {
                    request_receive<bool>(
                        *sys, conform_events_, broadcast::leave_broadcast_atom_v, as_actor());
                } catch (const std::exception &) {
                }
                conform_events_ = caf::actor();
            }
            try {
                conform_events_ =
                    request_receive<caf::actor>(*sys, conform_manager, get_event_group_atom_v);
                request_receive<bool>(
                    *sys, conform_events_, broadcast::join_broadcast_atom_v, as_actor());
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }

        // setModelData(tree);
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }

    set_message_handler([=](caf::actor_companion * /*self*/) -> caf::message_handler {
        return {
            [=](utility::event_atom,
                conform::conform_tasks_atom,
                const std::vector<std::string> &) {},

            [=](utility::event_atom,
                json_store::sync_atom,
                const Uuid &uuid,
                const JsonStore &event) {
                try {
                    if (uuid == conform_uuid_)
                        receiveEvent(event);

                    // spdlog::warn("{}", modelData().dump(2));
                } catch (const std::exception &err) {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                }
            },
            [=](broadcast::broadcast_down_atom, const caf::actor_addr &) {},
            [=](const group_down_msg &g) {
                caf::aout(self())
                    << "ConformEngineUI down: " << to_string(g.source) << std::endl;
            }};
    });
}


QVariant ConformEngineUI::data(const QModelIndex &index, int role) const {
    auto result = QVariant();

    try {
        const auto &j = indexToData(index);

        switch (role) {
        case Roles::nameRole:
        case Qt::DisplayRole:
            try {
                if (j.count("name"))
                    result = QString::fromStdString(j.at("name"));
            } catch (...) {
            }
            break;

        default:
            result = JSONTreeModel::data(index, role);
            break;
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {} {} {}", __PRETTY_FUNCTION__, err.what(), role, index.row());
    }

    return result;
}

QFuture<bool> ConformEngineUI::conformPrepareSequenceFuture(const QModelIndex &sequenceIndex) {
    auto future = QFuture<bool>();

    try {
        if (not sequenceIndex.isValid())
            throw std::runtime_error("Invalid Sequence");
        if (sequenceIndex.data(SessionModel::Roles::typeRole).toString() != QString("Timeline"))
            throw std::runtime_error("Invalid Sequence type");

        auto sequence_actor = actorFromString(
            system(),
            StdFromQString(sequenceIndex.data(SessionModel::Roles::actorRole).toString()));
        auto sequence_uuid =
            UuidFromQUuid(sequenceIndex.data(SessionModel::Roles::actorUuidRole).toUuid());

        if (not sequence_actor)
            throw std::runtime_error("Invalid sequence actor");

        future = QtConcurrent::run([=]() {
            auto result = false;

            // populate data into conform request.. ?
            // or should conformer do the heavy lifting..
            auto conform_manager =
                system().registry().template get<caf::actor>(conform_registry);
            scoped_actor sys{system()};

            // always inserts next to old items ?
            // how would this work with timelines ?
            try {
                result = request_receive<bool>(
                    *sys,
                    conform_manager,
                    conform::conform_atom_v,
                    UuidActor(sequence_uuid, sequence_actor));

            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }

            return result;
        });

    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        future = QtConcurrent::run([=]() {
            auto result = false;
            return result;
        });
    }

    return future;
}

QFuture<QList<QUuid>> ConformEngineUI::conformItemsFuture(
    const QString &qtask,
    const QModelIndex &container,
    const QModelIndex &item,
    const bool fanOut,
    const bool removeSource) {
    const auto task = StdFromQString(qtask);

    // get container detail
    QModelIndexList items;

    // maybe media or clip, or track
    auto item_type = StdFromQString(item.data(SessionModel::Roles::typeRole).toString());

    if (item_type == "Media" or item_type == "Clip") {
        items.push_back(item);
    } else if (item_type == "Video Track" or item_type == "Audio Track") {
        item_type = "Clip";
        // iterate of track clips, adding those that are not locked.
        for (int i = 0; i < item.model()->rowCount(item); i++) {
            auto ind = item.model()->index(i, 0, item);

            if (ind.isValid() and
                ind.data(SessionModel::Roles::typeRole).toString() == "Clip" and
                ind.data(SessionModel::Roles::lockedRole).toBool() == false) {
                items.push_back(ind);
            }
        }
    }

    auto smodel =
        qobject_cast<SessionModel *>(const_cast<QAbstractItemModel *>(container.model()));
    auto jmodel = qobject_cast<JSONTreeModel *>(smodel);

    auto cactor = actorFromString(
        system(), StdFromQString(container.data(SessionModel::Roles::actorRole).toString()));
    auto cuuid = UuidFromQUuid(container.data(SessionModel::Roles::actorUuidRole).toUuid());

    auto playlist_index = smodel->getPlaylistIndex(container);

    auto pactor = actorFromString(
        system(),
        StdFromQString(playlist_index.data(SessionModel::Roles::actorRole).toString()));
    auto puuid =
        UuidFromQUuid(playlist_index.data(SessionModel::Roles::actorUuidRole).toUuid());

    auto item_uav  = UuidActorVector();
    auto before_uv = UuidVector();

    for (const auto &i : items) {
        auto media_index = QModelIndex();

        auto iactor = actorFromString(
            system(), StdFromQString(i.data(SessionModel::Roles::actorRole).toString()));

        auto iuuid = Uuid();
        if (item_type == "Media") {
            media_index = i;
            iuuid       = UuidFromQUuid(i.data(SessionModel::Roles::actorUuidRole).toUuid());
        } else if (item_type == "Clip") {
            // get media actor from clip..
            // there MAY NOT BE ONE..
            // get meda uuid..
            iuuid      = UuidFromQUuid(i.data(JSONTreeModel::Roles::idRole).toUuid());
            auto muuid = UuidFromQUuid(i.data(SessionModel::Roles::clipMediaUuidRole).toUuid());
            // find media actor
            if (not muuid.is_null()) {
                // find in playlist
                auto mlist = container.model()->index(0, 0, container);
                if (mlist.isValid()) {
                    media_index = jmodel->searchRecursive(
                        item.data(SessionModel::Roles::clipMediaUuidRole),
                        SessionModel::Roles::actorUuidRole,
                        mlist,
                        0,
                        0);
                }
            }
        }
        // get uuid of next media..
        auto buuid = Uuid();
        if (media_index.isValid()) {
            auto next_index =
                media_index.model()->sibling(media_index.row() + 1, 0, media_index);
            if (next_index.isValid())
                buuid =
                    UuidFromQUuid(next_index.data(SessionModel::Roles::actorUuidRole).toUuid());
        }

        if (not iuuid.is_null()) {
            item_uav.emplace_back(UuidActor(iuuid, iactor));
            before_uv.push_back(buuid);
        }
    }

    // spdlog::warn(
    //     "ConformEngineUI::conformItemsFuture task {} p {} {} c {} {} {} {} {} {}",
    //     task,
    //     to_string(puuid),
    //     to_string(pactor),
    //     to_string(cuuid),
    //     to_string(cactor),
    //     item_type,
    //     to_string(iuuid),
    //     to_string(iactor),
    //     remove);

    if (item_uav.size() > 1 and fanOut) {
        return QtConcurrent::run([=]() {
            auto result = QList<QUuid>();

            auto pending = std::vector<QFuture<QList<QUuid>>>();

            for (size_t i = 0; i < item_uav.size(); i++) {
                pending.emplace_back(conformItemsFuture(
                    task,
                    utility::UuidActorVector({item_uav.at(i)}),
                    UuidActor(puuid, pactor),
                    UuidActor(cuuid, cactor),
                    item_type,
                    utility::UuidVector({before_uv.at(i)}),
                    removeSource));
            }

            for (auto &i : pending) {
                auto r = i.result();
                result.append(r);
            }

            return result;
        });
    }

    return conformItemsFuture(
        task,
        item_uav,
        UuidActor(puuid, pactor),
        UuidActor(cuuid, cactor),
        item_type,
        before_uv,
        removeSource);
}


QFuture<QList<QUuid>> ConformEngineUI::conformToSequenceFuture(
    const QModelIndex &_playlistIndex,
    const QModelIndexList &_mediaIndexes,
    const QModelIndex &sequenceIndex,
    const QModelIndex &conformTrackIndex,
    const bool replace,
    const QString &newTrackName) {

    auto playlistIndex = _playlistIndex;
    auto mediaIndexes  = _mediaIndexes;

    auto future = QFuture<QList<QUuid>>();

    try {
        if (mediaIndexes.empty())
            throw std::runtime_error("Empty media list");

        if (not playlistIndex.isValid())
            throw std::runtime_error("Invalid Playlist");
        if (playlistIndex.data(SessionModel::Roles::typeRole).toString() != QString("Playlist"))
            throw std::runtime_error("Invalid Playlist type");


        if (not sequenceIndex.isValid())
            throw std::runtime_error("Invalid Sequence");
        if (sequenceIndex.data(SessionModel::Roles::typeRole).toString() != QString("Timeline"))
            throw std::runtime_error("Invalid Sequence type");


        auto conformTrackUuidActor = UuidActor();

        if (conformTrackIndex.isValid()) {
            auto ctuuid =
                UuidFromQUuid(conformTrackIndex.data(JSONTreeModel::Roles::idRole).toUuid());
            auto ctactor = actorFromString(
                system(),
                StdFromQString(
                    conformTrackIndex.data(SessionModel::Roles::actorRole).toString()));

            if (ctactor and not ctuuid.is_null())
                conformTrackUuidActor = UuidActor(ctuuid, ctactor);
        }

        // safe ??
        auto sessionModel = qobject_cast<SessionModel *>(
            const_cast<QAbstractItemModel *>(playlistIndex.model()));
        auto sequencePlaylistIndex = sessionModel->getPlaylistIndex(sequenceIndex);

        if (sequencePlaylistIndex != playlistIndex) {
            playlistIndex = sequencePlaylistIndex;
            mediaIndexes  = sessionModel->copyRows(
                mediaIndexes,
                sessionModel->rowCount(sessionModel->index(0, 0, playlistIndex)),
                playlistIndex);


            for (const auto &i : mediaIndexes) {
                // wait for them to be valid..
                while (i.data(SessionModel::Roles::placeHolderRole).toBool() == true) {
                    QCoreApplication::processEvents(
                        QEventLoop::WaitForMoreEvents | QEventLoop::ExcludeUserInputEvents, 50);
                }
            }
        }

        auto playlist_actor = actorFromString(
            system(),
            StdFromQString(playlistIndex.data(SessionModel::Roles::actorRole).toString()));
        auto playlist_uuid =
            UuidFromQUuid(playlistIndex.data(SessionModel::Roles::actorUuidRole).toUuid());

        auto media_list = utility::UuidActorVector();
        for (const auto &i : mediaIndexes) {
            auto media_actor = actorFromString(
                system(), StdFromQString(i.data(SessionModel::Roles::actorRole).toString()));
            auto media_uuid =
                UuidFromQUuid(i.data(SessionModel::Roles::actorUuidRole).toUuid());
            if (media_actor)
                media_list.emplace_back(UuidActor(media_uuid, media_actor));
        }

        if (not playlist_actor)
            throw std::runtime_error("Invalid playlist actor");

        auto sequence_actor = actorFromString(
            system(),
            StdFromQString(sequenceIndex.data(SessionModel::Roles::actorRole).toString()));
        auto sequence_uuid =
            UuidFromQUuid(sequenceIndex.data(SessionModel::Roles::actorUuidRole).toUuid());

        if (not sequence_actor)
            throw std::runtime_error("Invalid sequence actor");

        future = QtConcurrent::run([=]() {
            auto result = QList<QUuid>();

            // populate data into conform request.. ?
            // or should conformer do the heavy lifting..
            auto conform_manager =
                system().registry().template get<caf::actor>(conform_registry);
            scoped_actor sys{system()};

            // always inserts next to old items ?
            // how would this work with timelines ?
            try {
                auto operations                  = JsonStore(conform::ConformOperationsJSON);
                operations["create_media"]       = false;
                operations["remove_media"]       = false;
                operations["insert_media"]       = true;
                operations["replace_clip"]       = replace;
                operations["new_track_name"]     = StdFromQString(newTrackName);
                operations["remove_failed_clip"] = true;

                auto reply = request_receive<conform::ConformReply>(
                    *sys,
                    conform_manager,
                    conform::conform_atom_v,
                    operations,
                    UuidActor(playlist_uuid, playlist_actor),
                    UuidActor(sequence_uuid, sequence_actor),
                    conformTrackUuidActor,
                    media_list);

                for (const auto &i : reply.items_) {
                    if (i) {
                        for (const auto &j : *i)
                            result.push_back(QUuidFromUuid(std::get<0>(j)));
                    }
                }
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }

            return result;
        });

    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        future = QtConcurrent::run([=]() {
            auto result = QList<QUuid>();
            return result;
        });
    }

    return future;
}


QFuture<QList<QUuid>> ConformEngineUI::conformItemsFuture(
    const std::string &task,
    const utility::UuidActorVector &items,
    const utility::UuidActor &playlist,
    const utility::UuidActor &container,
    const std::string &item_type,
    const utility::UuidVector &before,
    const bool removeSource) {

    return QtConcurrent::run([=]() {
        auto result = QList<QUuid>();

        if (items.empty())
            return result;

        // populate data into conform request.. ?
        // or should conformer do the heavy lifting..
        auto conform_manager = system().registry().template get<caf::actor>(conform_registry);
        scoped_actor sys{system()};

        // always inserts next to old items ?
        // how would this work with timelines ?
        try {
            auto operations                  = JsonStore(conform::ConformOperationsJSON);
            operations["create_media"]       = true;
            operations["insert_media"]       = true;
            operations["replace_clip"]       = true;
            operations["remove_media"]       = removeSource;
            operations["remove_failed_clip"] = removeSource;

            auto reply = request_receive<conform::ConformReply>(
                *sys,
                conform_manager,
                conform::conform_atom_v,
                task,
                operations,
                playlist,
                container,
                item_type,
                items,
                before);

            for (const auto &i : reply.items_) {
                if (i) {
                    for (const auto &j : *i) {
                        result.push_back(QUuidFromUuid(std::get<0>(j)));
                    }
                }
            }

        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }

        return result;
    });
}
