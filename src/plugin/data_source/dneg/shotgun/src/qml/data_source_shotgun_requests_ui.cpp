// SPDX-License-Identifier: Apache-2.0
#include "data_source_shotgun_ui.hpp"
#include "shotgun_model_ui.hpp"

#include "../data_source_shotgun.hpp"
#include "../data_source_shotgun_definitions.hpp"
#include "../data_source_shotgun_query_engine.hpp"

#include "xstudio/atoms.hpp"

using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::shotgun_client;
using namespace xstudio::ui::qml;

#define REQUEST_BEGIN() return QtConcurrent::run([=]() { \
        if (backend_) { \
            try {

#define REQUEST_END()                                                                          \
    }                                                                                          \
    catch (const XStudioError &err) {                                                          \
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());                                \
        auto error                = R"({'error':{})"_json;                                     \
        error["error"]["source"]  = to_string(err.type());                                     \
        error["error"]["message"] = err.what();                                                \
        return QStringFromStd(JsonStore(error).dump());                                        \
    }                                                                                          \
    catch (const std::exception &err) {                                                        \
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());                                \
        return QStringFromStd(err.what());                                                     \
    }                                                                                          \
    }                                                                                          \
    return QString();                                                                          \
    });


QFuture<QString> ShotgunDataSourceUI::loadPresetModelFuture() {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto request    = JsonStore(GetData);
    request["type"] = "preset_model";

    auto data = R"({"uuid":null, "data":null })"_json;

    auto uuid = request_receive<UuidVector>(*sys, backend_, json_store::sync_atom_v)[0];

    data["uuid"] = uuid;

    auto jsn = request_receive<JsonStore>(*sys, backend_, json_store::sync_atom_v, uuid);

    data["data"] = jsn;

    anon_send(as_actor(), shotgun_info_atom_v, request, JsonStore(data));

    return QStringFromStd(data.dump());

    REQUEST_END()
}


QFuture<QString> ShotgunDataSourceUI::getDataFuture(const QString &type, const int project_id) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto request          = JsonStore(GetData);
    request["type"]       = StdFromQString(type);
    request["project_id"] = project_id;

    auto data = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, get_data_atom_v, request);

    anon_send(as_actor(), shotgun_info_atom_v, request, data);

    return QStringFromStd(data.dump());

    REQUEST_END()
}


QFuture<QString> ShotgunDataSourceUI::getProjectsFuture() {
    return getDataFuture(QString("project"));
}

QFuture<QString> ShotgunDataSourceUI::getSchemaFieldsFuture(const QString &type) {
    return getDataFuture(type);
}

// get json of versions data from shotgun.
QFuture<QString>
ShotgunDataSourceUI::getVersionsFuture(const int project_id, const QVariant &qids) {

    std::vector<int> ids;
    for (const auto &i : qids.toList())
        ids.push_back(i.toInt());

    //    return QtConcurrent::run([=, project_id = project_id]() {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto filter = R"(
    {
        "logical_operator": "and",
        "conditions": [
            ["project", "is", {"type":"Project", "id":0}],
            ["id", "in", []]
        ]
    })"_json;

    filter["conditions"][0][2]["id"] = project_id;
    for (const auto i : ids)
        filter["conditions"][1][2].push_back(i);

    // we've got more that 5000 employees....
    auto data = request_receive_wait<JsonStore>(
        *sys,
        backend_,
        SHOTGUN_TIMEOUT,
        shotgun_entity_search_atom_v,
        "Versions",
        JsonStore(filter),
        VersionFields,
        std::vector<std::string>({"id"}),
        1,
        4999);

    if (not data.count("data"))
        throw std::runtime_error(data.dump(2));

    // reorder results based on request order..
    std::map<int, JsonStore> result_items;
    for (const auto &i : data["data"])
        result_items[i.at("id").get<int>()] = i;

    data["data"].clear();
    for (const auto i : ids) {
        if (result_items.count(i))
            data["data"].push_back(result_items[i]);
    }

    return QStringFromStd(data.dump());

    REQUEST_END()
}


QFuture<QString> ShotgunDataSourceUI::getPlaylistLinkMediaFuture(const QUuid &playlist) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req             = JsonStore(GetLinkMedia);
    req["playlist_uuid"] = to_string(UuidFromQUuid(playlist));

    return QStringFromStd(
        request_receive_wait<JsonStore>(
            *sys, backend_, SHOTGUN_TIMEOUT, data_source::get_data_atom_v, req)
            .dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::getPlaylistValidMediaCountFuture(const QUuid &playlist) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req             = JsonStore(GetValidMediaCount);
    req["playlist_uuid"] = to_string(UuidFromQUuid(playlist));

    return QStringFromStd(
        request_receive_wait<JsonStore>(
            *sys, backend_, SHOTGUN_TIMEOUT, data_source::get_data_atom_v, req)
            .dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::getGroupsFuture(const int project_id) {
    return getDataFuture(QString("group"), project_id);
}

QFuture<QString> ShotgunDataSourceUI::getSequencesFuture(const int project_id) {
    return getDataFuture(QString("sequence"), project_id);
}

QFuture<QString> ShotgunDataSourceUI::getPlaylistsFuture(const int project_id) {
    return getDataFuture(QString("playlist"), project_id);
}


QFuture<QString> ShotgunDataSourceUI::getShotsFuture(const int project_id) {
    return getDataFuture(QString("shot"), project_id);
}


QFuture<QString> ShotgunDataSourceUI::getUsersFuture() {
    return getDataFuture(QString("user"));
}

QFuture<QString> ShotgunDataSourceUI::getDepartmentsFuture() {
    return getDataFuture(QString("department"));
}

QFuture<QString> ShotgunDataSourceUI::getReferenceTagsFuture() {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto filter = R"(
    {
        "logical_operator": "and",
        "conditions": [
            ["name", "ends_with", ".REFERENCE"]
        ]
    })"_json;

    // we've got more that 5000 employees....
    auto data = request_receive_wait<JsonStore>(
        *sys,
        backend_,
        SHOTGUN_TIMEOUT,
        shotgun_entity_search_atom_v,
        "Tags",
        JsonStore(filter),
        std::vector<std::string>({"name", "id"}),
        std::vector<std::string>({"name"}),
        1,
        4999);

    if (not data.count("data"))
        throw std::runtime_error(data.dump(2));

    for (auto &i : data["data"]) {
        auto str                = i["attributes"]["name"].get<std::string>();
        i["attributes"]["name"] = str.substr(0, str.size() - sizeof(".REFERENCE") + 1);
    }

    anon_send(
        as_actor(), shotgun_info_atom_v, JsonStore(R"({"type": "reference_tag"})"_json), data);

    return QStringFromStd(data.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::getCustomEntity24Future(const int project_id) {
    return getDataFuture(QString("unit"), project_id);
}

QFuture<QString> ShotgunDataSourceUI::addVersionToPlaylistFuture(
    const QString &version, const QUuid &playlist, const QUuid &before) {

    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto media = request_receive<UuidActorVector>(
        *sys,
        backend_,
        playlist::add_media_atom_v,
        JsonStore(nlohmann::json::parse(StdFromQString(version))),
        UuidFromQUuid(playlist),
        caf::actor(),
        UuidFromQUuid(before));
    auto result = nlohmann::json::array();
    // return uuids..
    for (const auto &i : media) {
        result.push_back(i.uuid());
    }

    return QStringFromStd(JsonStore(result).dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::updateEntityFuture(
    const QString &entity, const int record_id, const QString &update_json) {

    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto js = request_receive_wait<JsonStore>(
        *sys,
        backend_,
        SHOTGUN_TIMEOUT,
        shotgun_update_entity_atom_v,
        StdFromQString(entity),
        record_id,
        utility::JsonStore(nlohmann::json::parse(StdFromQString(update_json))));
    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::preparePlaylistNotesFuture(
    const QUuid &playlist,
    const QList<QUuid> &media,
    const bool notify_owner,
    const std::vector<int> notify_group_ids,
    const bool combine,
    const bool add_time,
    const bool add_playlist_name,
    const bool add_type,
    const bool anno_requires_note,
    const bool skip_already_published,
    const QString &defaultType) {

    return QtConcurrent::run([=]() {
        if (backend_) {
            try {
                scoped_actor sys{system()};
                auto req = JsonStore(GetPrepareNotes);

                for (const auto &i : media)
                    req["media_uuids"].push_back(to_string(UuidFromQUuid(i)));

                req["playlist_uuid"]          = to_string(UuidFromQUuid(playlist));
                req["notify_owner"]           = notify_owner;
                req["notify_group_ids"]       = notify_group_ids;
                req["combine"]                = combine;
                req["add_time"]               = add_time;
                req["add_playlist_name"]      = add_playlist_name;
                req["add_type"]               = add_type;
                req["anno_requires_note"]     = anno_requires_note;
                req["skip_already_published"] = skip_already_published;
                req["default_type"]           = StdFromQString(defaultType);

                auto js = request_receive_wait<JsonStore>(
                    *sys, backend_, SHOTGUN_TIMEOUT, data_source::get_data_atom_v, req);
                return QStringFromStd(js.dump());
            } catch (const XStudioError &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                auto error = R"({'error':{})"_json;
                // error["error"]["source"] = to_string(err.type());
                // error["error"]["message"] = err.what();
                return QStringFromStd(JsonStore(error).dump());
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                return QStringFromStd(err.what());
            }
        }
        return QString();
    });
}


QFuture<QString>
ShotgunDataSourceUI::pushPlaylistNotesFuture(const QString &notes, const QUuid &playlist) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req             = JsonStore(PostCreateNotes);
    req["playlist_uuid"] = to_string(UuidFromQUuid(playlist));
    req["payload"]       = JsonStore(nlohmann::json::parse(StdFromQString(notes))["payload"]);

    auto js = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::post_data_atom_v, req);

    return QStringFromStd(js.dump());

    REQUEST_END()
}


QFuture<QString> ShotgunDataSourceUI::createPlaylistFuture(
    const QUuid &playlist,
    const int project_id,
    const QString &name,
    const QString &location,
    const QString &playlist_type) {

    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req             = JsonStore(PostCreatePlaylist);
    req["playlist_uuid"] = to_string(UuidFromQUuid(playlist));
    req["project_id"]    = project_id;
    req["code"]          = StdFromQString(name);
    req["location"]      = StdFromQString(location);
    req["playlist_type"] = StdFromQString(playlist_type);

    auto js = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::post_data_atom_v, req);
    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::updatePlaylistVersionsFuture(const QUuid &playlist) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req             = JsonStore(PutUpdatePlaylistVersions);
    req["playlist_uuid"] = to_string(UuidFromQUuid(playlist));
    auto js              = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::put_data_atom_v, req);
    return QStringFromStd(js.dump());

    REQUEST_END()
}

// find playlist id from playlist
// request versions from shotgun
// add to playlist.
QFuture<QString> ShotgunDataSourceUI::refreshPlaylistVersionsFuture(const QUuid &playlist) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req             = JsonStore(UseRefreshPlaylist);
    req["playlist_uuid"] = to_string(UuidFromQUuid(playlist));
    auto js              = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::use_data_atom_v, req);
    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::getPlaylistNotesFuture(const int id) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto note_filter = R"(
    {
        "logical_operator": "and",
        "conditions": [
            ["note_links", "in", {"type":"Playlist", "id":0}]
        ]
    })"_json;

    note_filter["conditions"][0][2]["id"] = id;

    auto order = request_receive_wait<JsonStore>(
        *sys,
        backend_,
        SHOTGUN_TIMEOUT,
        shotgun_entity_search_atom_v,
        "Notes",
        JsonStore(note_filter),
        std::vector<std::string>({"*"}),
        std::vector<std::string>(),
        1,
        4999);

    return QStringFromStd(order.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::getPlaylistVersionsFuture(const int id) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto vers = request_receive_wait<JsonStore>(
        *sys,
        backend_,
        SHOTGUN_TIMEOUT,
        shotgun_entity_atom_v,
        "Playlists",
        id,
        std::vector<std::string>());

    // spdlog::warn("{}", vers.dump(2));

    auto order_filter = R"(
    {
        "logical_operator": "and",
        "conditions": [
            ["playlist", "is", {"type":"Playlist", "id":0}]
        ]
    })"_json;

    order_filter["conditions"][0][2]["id"] = id;

    auto order = request_receive_wait<JsonStore>(
        *sys,
        backend_,
        SHOTGUN_TIMEOUT,
        shotgun_entity_search_atom_v,
        "PlaylistVersionConnection",
        JsonStore(order_filter),
        std::vector<std::string>({"sg_sort_order", "version"}),
        std::vector<std::string>({"sg_sort_order"}),
        1,
        4999);

    // should be returned in the correct order..

    // spdlog::warn("{}", order.dump(2));

    std::vector<std::string> version_ids;
    for (const auto &i : order["data"])
        version_ids.emplace_back(
            std::to_string(i["relationships"]["version"]["data"].at("id").get<int>()));

    if (version_ids.empty())
        return QStringFromStd(vers.dump());

    // expand version information..
    // get versions
    auto query       = R"({})"_json;
    auto chunked_ids = split_vector(version_ids, 100);
    auto data        = R"([])"_json;

    for (const auto &chunk : chunked_ids) {
        query["id"] = join_as_string(chunk, ",");

        auto js = request_receive_wait<JsonStore>(
            *sys,
            backend_,
            SHOTGUN_TIMEOUT,
            shotgun_entity_filter_atom_v,
            "Versions",
            JsonStore(query),
            VersionFields,
            std::vector<std::string>(),
            1,
            4999);
        // reorder list based on playlist..
        // spdlog::warn("{}", js.dump(2));

        for (const auto &i : chunk) {
            for (auto &j : js["data"]) {

                // spdlog::warn("{} {}", std::to_string(j["id"].get<int>()), i);
                if (std::to_string(j["id"].get<int>()) == i) {
                    data.push_back(j);
                    break;
                }
            }
        }
    }

    auto data_tmp    = R"({"data":[]})"_json;
    data_tmp["data"] = data;

    // spdlog::warn("{}", js.dump(2));

    // add back in
    vers["data"]["relationships"]["versions"] = data_tmp;

    // create playlist..
    return QStringFromStd(vers.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::tagEntityFuture(
    const QString &entity, const int record_id, const int tag_id) {

    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto req = JsonStore(PostTagEntity);

    req["entity"]    = StdFromQString(entity);
    req["entity_id"] = record_id;
    req["tag_id"]    = tag_id;

    auto js = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::post_data_atom_v, req);

    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::renameTagFuture(const int tag_id, const QString &text) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto req = JsonStore();

    if (tag_id) {
        req           = JsonStore(PostRenameTag);
        req["tag_id"] = tag_id;
        req["value"]  = StdFromQString(text);
    } else {
        req          = JsonStore(PostCreateTag);
        req["value"] = StdFromQString(text);
    }

    auto js = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::post_data_atom_v, req);

    // trigger update to get new tag.
    getReferenceTagsFuture();

    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::removeTagFuture(const int tag_id) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto js = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, shotgun_delete_entity_atom_v, "Tag", tag_id);

    // trigger update to get new tag.
    getReferenceTagsFuture();

    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::untagEntityFuture(
    const QString &entity, const int record_id, const int tag_id) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};

    auto req = JsonStore(PostUnTagEntity);

    req["entity"]    = StdFromQString(entity);
    req["entity_id"] = record_id;
    req["tag_id"]    = tag_id;

    auto js = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::post_data_atom_v, req);

    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::createTagFuture(const QString &text) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req     = JsonStore(PostCreateTag);
    req["value"] = StdFromQString(text);

    auto js = request_receive_wait<JsonStore>(
        *sys, backend_, SHOTGUN_TIMEOUT, data_source::post_data_atom_v, req);

    // trigger update to get new tag.
    getReferenceTagsFuture();

    return QStringFromStd(js.dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::getEntityFuture(const QString &qentity, const int id) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto entity = StdFromQString(qentity);
    std::vector<std::string> fields;

    if (entity == "Version") {
        fields = VersionFields;
    }

    return QStringFromStd(
        request_receive_wait<JsonStore>(
            *sys, backend_, SHOTGUN_TIMEOUT, shotgun_entity_atom_v, entity, id, fields)
            .dump());

    REQUEST_END()
}

QFuture<QString> ShotgunDataSourceUI::addDownloadToMediaFuture(const QUuid &media) {
    REQUEST_BEGIN()

    scoped_actor sys{system()};
    auto req          = JsonStore(GetDownloadMedia);
    req["media_uuid"] = to_string(UuidFromQUuid(media));

    return QStringFromStd(
        request_receive_wait<JsonStore>(
            *sys, backend_, SHOTGUN_TIMEOUT, data_source::get_data_atom_v, req)
            .dump());

    REQUEST_END()
}
