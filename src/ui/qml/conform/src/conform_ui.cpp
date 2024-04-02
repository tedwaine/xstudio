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
                    //                    spdlog::warn("{}", event.dump(2));
                    if (uuid == conform_uuid_)
                        receiveEvent(event);
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


QFuture<QList<QUuid>> ConformEngineUI::conformMediaFuture(
    const QString &qtask,
    const QModelIndex &playlist,
    const QModelIndex &item,
    const bool remove) {
    const auto task = StdFromQString(qtask);

    // maybe media or clip
    auto item_type = StdFromQString(item.data(SessionModel::Roles::typeRole).toString());
    auto mactor = caf::actor();
    auto muuid = utility::Uuid();

    if(item_type == "Media") {
        mactor = actorFromString(
            system(), StdFromQString(item.data(SessionModel::Roles::actorRole).toString()));
        muuid = UuidFromQUuid(item.data(SessionModel::Roles::actorUuidRole).toUuid());
    } else if(item_type == "Clip") {
        // get media actor from clip..
        // there MAY NOT BE ONE..
        // get meda uuid..
        muuid = UuidFromQUuid(item.data(SessionModel::Roles::clipMediaUuidRole).toUuid());
        // find media actor
        if(not muuid.is_null()) {
            // find in playlist
            auto mlist = playlist.model()->index(0, 0, playlist);
            if(mlist.isValid()) {

                auto model = const_cast<QAbstractItemModel *>(mlist.model());
                auto jmodel = qobject_cast<JSONTreeModel *>(model);

                auto media_index = jmodel->searchRecursive(
                    item.data(SessionModel::Roles::clipMediaUuidRole),
                    SessionModel::Roles::actorUuidRole,
                    mlist,
                    0,
                    0
                );
                if(media_index.isValid()) {
                    mactor = actorFromString(
                        system(), StdFromQString(media_index.data(SessionModel::Roles::actorRole).toString()));
                }
            }
        }
    }

    auto pactor = actorFromString(
        system(), StdFromQString(playlist.data(SessionModel::Roles::actorRole).toString()));
    auto puuid = UuidFromQUuid(playlist.data(SessionModel::Roles::actorUuidRole).toUuid());

    // get uuid of next media..
    auto buuid = Uuid();
    auto next_index = item.model()->sibling(item.row() + 1, 0, item);
    if (next_index.isValid()) {
        buuid = UuidFromQUuid(next_index.data(SessionModel::Roles::actorUuidRole).toUuid());
    }

    spdlog::warn(
        "ConformEngineUI::conformMediaFuture {} {} {} {} {} {} {}",
        task,
        to_string(puuid),
        to_string(pactor),
        item_type,
        to_string(muuid),
        to_string(mactor),
        remove);


    return QtConcurrent::run([=]() {
        auto result = QList<QUuid>();

        if(not mactor)
            return result;

        // populate data into conform request.. ?
        // or should conformer do the heavy lifting..
        auto conform_manager = system().registry().template get<caf::actor>(conform_registry);
        scoped_actor sys{system()};

        // always inserts next to old items ?
        // how would this work with timelines ?

        auto reply = request_receive<conform::ConformReply>(
            *sys,
            conform_manager,
            conform::conform_atom_v,
            task,
            utility::JsonStore(),
            UuidActor(puuid, pactor),
            UuidActorVector({UuidActor(muuid, mactor)}),
            UuidVector({buuid}));

        for (const auto &i : reply.items_) {
            if (i) {
                for (const auto &j : *i) {
                    result.push_back(QUuidFromUuid(std::get<0>(j)));
                }
            }
        }

        if (remove and not result.empty()) {
            request_receive<bool>(*sys, pactor, playlist::remove_media_atom_v, muuid);
        }

        return result;
    });
}
