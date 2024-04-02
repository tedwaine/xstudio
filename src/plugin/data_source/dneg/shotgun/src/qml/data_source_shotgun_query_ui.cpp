// SPDX-License-Identifier: Apache-2.0
#include "data_source_shotgun_ui.hpp"
#include "shotgun_model_ui.hpp"

#include "../data_source_shotgun.hpp"
#include "../data_source_shotgun_definitions.hpp"
#include "../data_source_shotgun_query_engine.hpp"

// #include "xstudio/utility/string_helpers.hpp"
// #include "xstudio/ui/qml/json_tree_model_ui.hpp"
// #include "xstudio/global_store/global_store.hpp"
// #include "xstudio/atoms.hpp"
// #include "xstudio/ui/qml/module_ui.hpp"
// #include "xstudio/utility/chrono.hpp"

// #include <QProcess>
// #include <QQmlExtensionPlugin>
// #include <qdebug.h>

using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::shotgun_client;
using namespace xstudio::ui::qml;
using namespace xstudio::global_store;


void ShotgunDataSourceUI::updateQueryValueCache(
    const std::string &type, const utility::JsonStore &data, const int project_id) {

    if (type == "Sequence")
        QueryEngine::set_shot_sequence_lookup(
            QueryEngine::cache_name("ShotSequence", project_id), data, (*query_value_cache_));

    QueryEngine::set_lookup(
        QueryEngine::cache_name(type, project_id), data, (*query_value_cache_));
}

// merge global filters with Preset.
// Not sure if this should really happen here..
// DST = PRESET src == Global

QVariant ShotgunDataSourceUI::mergeQueries(
    const QVariant &dst, const QVariant &src, const bool ignore_duplicates) const {

    JsonStore dst_qry;
    JsonStore src_qry;

    try {
        if (std::string(dst.typeName()) == "QJSValue") {
            dst_qry = nlohmann::json::parse(
                QJsonDocument::fromVariant(dst.value<QJSValue>().toVariant())
                    .toJson(QJsonDocument::Compact)
                    .constData());
        } else {
            dst_qry = nlohmann::json::parse(
                QJsonDocument::fromVariant(dst).toJson(QJsonDocument::Compact).constData());
        }

        if (std::string(src.typeName()) == "QJSValue") {
            src_qry = nlohmann::json::parse(
                QJsonDocument::fromVariant(src.value<QJSValue>().toVariant())
                    .toJson(QJsonDocument::Compact)
                    .constData());
        } else {
            src_qry = nlohmann::json::parse(
                QJsonDocument::fromVariant(src).toJson(QJsonDocument::Compact).constData());
        }

        auto merged = QueryEngine::merge_query(
            dst_qry["queries"], src_qry.at("queries"), ignore_duplicates);
        dst_qry["queries"] = merged;

    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }

    return QVariantMapFromJson(dst_qry);
}

QFuture<QString> ShotgunDataSourceUI::executeQueryNew(
    const QStringList &preset_paths, const int project_id, const bool update_result_model) {

    return QtConcurrent::run([=]() {
        if (backend_) {
            scoped_actor sys{system()};

            std::vector<std::string> paths;

            for (const auto &i : preset_paths)
                paths.emplace_back(StdFromQString(i));

            auto request            = JsonStore(GetExecutePreset);
            request["project_id"]   = project_id;
            request["preset_paths"] = paths;
            request["metadata"]     = live_link_metadata_;
            request["context"]      = R"({
                "type": null,
                "epoc": null,
                "audio_source": "",
                "visual_source": "",
                "flag_text": "",
                "flag_colour": "",
                "truncated": false
            })"_json;

            request["context"]["epoc"] = utility::to_epoc_milliseconds(utility::clock::now());
            request["context"]["type"] = "note_result";
            try {

                auto data = request_receive_wait<JsonStore>(
                    *sys, backend_, SHOTGUN_TIMEOUT, get_data_atom_v, request);

                if (data.at("result").at("data").is_array())
                    data["context"]["truncated"] =
                        (static_cast<int>(data.at("result").at("data").size()) ==
                         data.at("max_result"));

                if (update_result_model)
                    anon_send(as_actor(), shotgun_info_atom_v, data);

                return QStringFromStd(data.dump());

            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                // silence error..
                if (update_result_model)
                    anon_send(as_actor(), shotgun_info_atom_v, request);

                if (starts_with(std::string(err.what()), "LiveLink ")) {
                    return QStringFromStd(request.dump()); // R"({"data":[]})");
                }

                return QStringFromStd(err.what());
            }
        }
        return QString();
    });
}


QFuture<QString> ShotgunDataSourceUI::executeQuery(
    const QString &context,
    const int project_id,
    const QVariant &query,
    const bool update_result_model) {
    // build and dispatch query, we then pass result via message back to ourself.
    JsonStore qry;

    try {
        qry = JsonStore(nlohmann::json::parse(
            QJsonDocument::fromVariant(query.value<QJSValue>().toVariant())
                .toJson(QJsonDocument::Compact)
                .constData()));

    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }

    return QtConcurrent::run([=]() {
        if (backend_ and not qry.is_null()) {
            scoped_actor sys{system()};

            try {
                std::string entity;
                auto query_context = R"({
                    "type": null,
                    "epoc": null,
                    "audio_source": "",
                    "visual_source": "",
                    "flag_text": "",
                    "flag_colour": "",
                    "truncated": false
                })"_json;

                query_context["epoc"] = utility::to_epoc_milliseconds(utility::clock::now());

                if (context == "Playlists") {
                    query_context["type"] = "playlist_result";
                    entity                = "Playlists";
                } else if (context == "Versions") {
                    query_context["type"] = "shot_result";
                    entity                = "Versions";
                } else if (context == "Reference") {
                    query_context["type"] = "reference_result";
                    entity                = "Versions";
                } else if (context == "Versions Tree") {
                    query_context["type"] = "shot_tree_result";
                    entity                = "Versions";
                } else if (context == "Menu Setup") {
                    query_context["type"] = "media_action_result";
                    entity                = "Versions";
                } else if (context == "Notes") {
                    query_context["type"] = "note_result";
                    entity                = "Notes";
                } else if (context == "Notes Tree") {
                    query_context["type"] = "note_tree_result";
                    entity                = "Notes";
                }

                auto request = QueryEngine::build_query(
                    project_id,
                    entity,
                    R"([])"_json,
                    qry.at("queries"),
                    query_context,
                    *query_value_cache_);

                // spdlog::warn("{}\n", request.dump(2));

                try {
                    auto data = request_receive_wait<JsonStore>(
                        *sys, backend_, SHOTGUN_TIMEOUT, get_data_atom_v, request);

                    if (data.at("result").at("data").is_array())
                        data["context"]["truncated"] =
                            (static_cast<int>(data.at("result").at("data").size()) ==
                             data.at("max_result"));

                    if (update_result_model)
                        anon_send(as_actor(), shotgun_info_atom_v, data);

                    return QStringFromStd(data.dump());

                } catch (const std::exception &err) {
                    spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
                    // silence error..
                    if (update_result_model)
                        anon_send(as_actor(), shotgun_info_atom_v, request);

                    if (starts_with(std::string(err.what()), "LiveLink ")) {
                        return QStringFromStd(request.dump()); // R"({"data":[]})");
                    }

                    return QStringFromStd(err.what());
                }
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }
        }
        return QString();
    });
}
