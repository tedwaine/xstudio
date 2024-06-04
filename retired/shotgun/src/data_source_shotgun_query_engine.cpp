// SPDX-License-Identifier: Apache-2.0

#include "data_source_shotgun_definitions.hpp"
#include "data_source_shotgun_query_engine.hpp"
#include "xstudio/atoms.hpp"
#include "xstudio/shotgun_client/shotgun_client.hpp"
#include "xstudio/utility/string_helpers.hpp"
#include "xstudio/utility/uuid.hpp"

using namespace xstudio;
using namespace xstudio::shotgun_client;
using namespace xstudio::utility;

template <typename T>
T QueryEngine::to_value(const nlohmann::json &jsn, const std::string &key, const T &fallback) {
    auto result = fallback;

    try {
        if (jsn.count(key) and not jsn.at(key).is_null())
            result = jsn.at(key).get<T>();
    } catch (...) {
    }

    return result;
}

void QueryEngine::initialise_presets() {
    auto user_tmp  = RootTemplate;
    user_tmp["id"] = Uuid::generate();
    user_presets_  = JsonStoreSync(user_tmp);

    system_presets_       = JsonStore(RootTemplate);
    system_presets_["id"] = Uuid::generate();
}

void QueryEngine::set_presets(
    const utility::JsonStore &user, const utility::JsonStore &system) {
    // purge children
    nlohmann::json tmp = user_presets_.as_json();

    tmp["children"]             = user;
    system_presets_["children"] = system;

    // we need to merge system presets into user
    merge_presets(tmp["children"], system_presets_["children"]);

    user_presets_.reset_data(tmp);
}

// iterate over system preset groups.
void QueryEngine::merge_presets(
    nlohmann::json &user_presets, const nlohmann::json &system_presets) {
    for (const auto &i : system_presets) {
        if (i.value("type", "") == "group")
            merge_group(user_presets, i);
        else
            spdlog::warn("{} Invalid group {}", __PRETTY_FUNCTION__, i.dump(2));
    }
}

void QueryEngine::merge_group(
    nlohmann::json &user_presets, const nlohmann::json &system_group) {
    // does group exist in user_presets..
    try {
        bool group_exists = false;
        auto group_id     = system_group.value("id", Uuid());

        for (auto &i : user_presets) {
            if (i.value("type", "") == "group" and i.value("id", Uuid()) == group_id) {
                group_exists = true;

                // update group name / entity?

                // validate group overrides
                if (not i.at("children").at(0).value("update", false) and
                    preset_diff(i.at("children").at(0), system_group.at("children").at(0))) {
                    // spdlog::warn("overrides differ");
                    // replace content.. as update flag not set by user change.
                    i["children"][0] = system_group.at("children").at(0);
                }

                // validate group presets.
                for (const auto &gp : system_group.at("children").at(1).at("children")) {
                    auto preset_id     = gp.value("id", Uuid());
                    bool preset_exists = false;

                    for (auto &up : i["children"][1]["children"]) {
                        if (up.value("type", "") == "preset" and
                            up.value("id", Uuid()) == preset_id) {
                            preset_exists = true;

                            // validate group preset
                            if (not up.value("update", false) and preset_diff(up, gp)) {
                                // replace content.. as update flag not set by user change.
                                up["children"] = gp.at("children");
                            }
                        }
                    }

                    if (not preset_exists)
                        i["children"][1]["children"].push_back(gp);
                }
            }
        }

        if (not group_exists) {
            // spdlog::warn("Add new group {}", system_group.value("name", ""));
            user_presets.push_back(system_group);
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }
}

bool QueryEngine::precache_needed(const int project_id, const utility::JsonStore &lookup) {
    // check expected keys exist..
    auto result = false;

    if (not lookup.count(cache_name("Department")))
        result = true;
    else if (not lookup.count(cache_name("User")))
        result = true;
    else if (not lookup.count(cache_name("Pipeline Status")))
        result = true;
    else if (not lookup.count(cache_name("Production Status")))
        result = true;
    else if (not lookup.count(cache_name("Shot Status")))
        result = true;
    else if (not lookup.count(cache_name("Unit", project_id)))
        result = true;
    else if (not lookup.count(cache_name("Playlist", project_id)))
        result = true;
    else if (not lookup.count(cache_name("Shot", project_id)))
        result = true;
    else if (not lookup.count(cache_name("Sequence", project_id)))
        result = true;

    return result;
}


bool QueryEngine::preset_diff(const nlohmann::json &a, const nlohmann::json &b) {
    auto result = true;

    try { // term count check (quick)
        if (a.at("children").size() == b.at("children").size()) {
            // term comparison..
            bool mismatch = false;

            for (const auto &ta : a.at("children")) {
                auto ta_id = ta.value("id", Uuid());
                mismatch   = true;

                for (const auto &tb : b.at("children")) {
                    if (tb.value("id", Uuid()) == ta_id) {
                        if (ta.at("term") == tb.at("term") and
                            ta.at("negated") == tb.at("negated") and
                            ta.at("enabled") == tb.at("enabled") and
                            ta.at("livelink") == tb.at("livelink") and
                            (ta.at("value") == tb.at("value") or
                             (not ta.at("livelink").is_null() and ta.value("livelink", false))))
                            mismatch = false;

                        break;
                    }
                }

                if (mismatch)
                    break;
            }

            if (not mismatch)
                result = false;
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }

    return result;
}

shotgun_client::FilterBy QueryEngine::terms_to_query(
    const JsonStore &terms,
    const int project_id,
    const std::string &entity,
    const utility::JsonStore &lookup,
    const bool and_mode,
    const bool initial) {

    auto result = FilterBy(BoolOperator::AND);

    if (initial) {
        // add terms we always want.
        result.emplace_back(Number("project.Project.id").is(project_id));

        if (entity == "Playlists") {
        } else if (entity == "Versions") {
            result.emplace_back(Text("sg_deleted").is_null());
            result.emplace_back(FilterBy().Or(
                Text("sg_path_to_movie").is_not_null(),
                Text("sg_path_to_frames").is_not_null()));
        } else if (entity == "Notes") {
        }
    } else if (not and_mode) {
        result = FilterBy(BoolOperator::OR);
    }

    for (const auto &i : terms) {
        if (i.at("term") == "operator") {
            result.emplace_back(terms_to_query(
                i.at("children"), project_id, entity, lookup, i.at("value") == "and", false));
        } else {
            try {
                add_term_to_filter(entity, i, project_id, lookup, &result);
            } catch (const std::exception &err) {
            }
        }
    }

    return result;
}

utility::JsonStore QueryEngine::apply_livelinks(
    const utility::JsonStore &terms,
    const utility::JsonStore &metadata,
    const utility::JsonStore &lookup) {

    auto result = utility::JsonStore(R"([])"_json);

    // apply livelinks..
    // update livelink data
    for (auto &i : terms) {
        auto t = i;
        if (t.at("term") == "operator") {
            t["children"] = apply_livelinks(t["children"], metadata, lookup);
        } else if (to_value(t, "livelink", false)) {
            auto linkvalue = get_livelink_value(t.value("term", ""), metadata, lookup);
            if (not linkvalue.is_null() and t["value"] != linkvalue)
                t["value"] = linkvalue;
        }
        result.emplace_back(t);
    }

    return result;
}

// handle expansion into OR/AND
utility::JsonStore QueryEngine::preprocess_terms(
    const utility::JsonStore &terms,
    const std::string &entity,
    utility::JsonStore &query,
    const bool and_mode,
    const bool initial) {
    auto result = utility::JsonStore(R"([])"_json);

    try {

        // special handling for top level.
        std::map<std::string, std::vector<utility::JsonStore>> dup_terms;
        std::vector<std::string> order_by;

        if (initial) {
            query["entity"] = entity;
            if (entity == "Versions")
                query["fields"] = VersionFields;
            else if (entity == "Notes")
                query["fields"] = NoteFields;
            else if (entity == "Playlists")
                query["fields"] = PlaylistFields;
        }

        for (const auto &i : terms) {
            if (i.value("enabled", true)) {
                auto term = i.value("term", "");
                if (term == "operator") {
                    auto op        = i;
                    op["children"] = preprocess_terms(
                        i.at("children"),
                        entity,
                        query,
                        i.value("value", "and") == "and",
                        false);
                    result.push_back(op);
                } else if (term == "Disable Global") {
                    // ignored
                } else if (term == "Result Limit") {
                    query["max_result"] = std::stoi(i.at("value").get<std::string>());
                } else if (term == "Preferred Visual") {
                    query["context"]["visual_source"] = i.at("value").get<std::string>();
                } else if (term == "Preferred Audio") {
                    query["context"]["audio_source"] = i.at("value").get<std::string>();
                } else if (term == "Flag Media") {
                    auto flag_text                = i.at("value").get<std::string>();
                    query["context"]["flag_text"] = flag_text;
                    if (flag_text == "Red")
                        query["context"]["flag_colour"] = "#FFFF0000";
                    else if (flag_text == "Green")
                        query["context"]["flag_colour"] = "#FF00FF00";
                    else if (flag_text == "Blue")
                        query["context"]["flag_colour"] = "#FF0000FF";
                    else if (flag_text == "Yellow")
                        query["context"]["flag_colour"] = "#FFFFFF00";
                    else if (flag_text == "Orange")
                        query["context"]["flag_colour"] = "#FFFFA500";
                    else if (flag_text == "Purple")
                        query["context"]["flag_colour"] = "#FF800080";
                    else if (flag_text == "Black")
                        query["context"]["flag_colour"] = "#FF000000";
                    else if (flag_text == "White")
                        query["context"]["flag_colour"] = "#FFFFFFFF";
                } else if (term == "Order By") {
                    auto val        = i.at("value").get<std::string>();
                    bool descending = false;

                    if (ends_with(val, " ASC")) {
                        val = val.substr(0, val.size() - 4);
                    } else if (ends_with(val, " DESC")) {
                        val        = val.substr(0, val.size() - 5);
                        descending = true;
                    }

                    std::string field = "";
                    // get sg term..
                    if (entity == "Playlists") {
                        if (val == "Date And Time")
                            field = "sg_date_and_time";
                        else if (val == "Created")
                            field = "created_at";
                        else if (val == "Updated")
                            field = "updated_at";
                    } else if (entity == "Versions") {
                        if (val == "Date And Time")
                            field = "created_at";
                        else if (val == "Created")
                            field = "created_at";
                        else if (val == "Updated")
                            field = "updated_at";
                        else if (val == "Client Submit")
                            field = "sg_date_submitted_to_client";
                        else if (val == "Version")
                            field = "sg_dneg_version";
                    } else if (entity == "Notes") {
                        if (val == "Created")
                            field = "created_at";
                        else if (val == "Updated")
                            field = "updated_at";
                    }

                    if (not field.empty())
                        order_by.push_back(descending ? "-" + field : field);
                } else {
                    // add normal term to map.
                    auto key = std::string(to_value(i, "negated", false) ? "Not " : "") + term;
                    if (not dup_terms.count(key))
                        dup_terms[key] = std::vector<utility::JsonStore>();

                    dup_terms[key].push_back(i);
                }
            }
        }

        // we've got list of terms in multi map
        // we now need to encapsulate in OR/AND groups.
        for (const auto &i : dup_terms) {
            if (i.second.size() == 1)
                result.push_back(i.second.front());
            else {
                auto mode = "";
                auto inverted =
                    starts_with(i.first, "Not ") or starts_with(i.first, "Exclude ");
                if (and_mode) {
                    if (inverted) {
                        mode = "and";
                    } else {
                        mode = "or";
                    }
                } else {
                    // not sure if it should be inverted ?
                    if (inverted) {
                        mode = "and";
                    } else {
                        mode = "or";
                    }
                }
                auto op        = JsonStore(OperatorTermTemplate);
                op["value"]    = mode;
                op["children"] = i.second;
                result.emplace_back(op);
            }
        }

        if (initial) {
            // set defaults if not specified
            if (query["context"]["visual_source"].empty())
                query["context"]["visual_source"] = "SG Movie";
            if (query["context"]["audio_source"].empty())
                query["context"]["audio_source"] = query["context"]["visual_source"];

            // set order by
            if (order_by.empty()) {
                order_by.emplace_back("-created_at");
            }

            query["order"] = order_by;
        }

    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        throw;
    }


    return result;
}

utility::JsonStore QueryEngine::build_query_new(
    const int project_id,
    const std::string &entity,
    const utility::JsonStore &group_terms,
    const utility::JsonStore &terms,
    const utility::JsonStore &context,
    const utility::JsonStore &metadata,
    const utility::JsonStore &lookup) {
    auto query = utility::JsonStore(GetQueryResult);

    query["context"] = context;

    auto merged_preset = utility::JsonStore(terms.at(0));
    for (size_t i = 1; i < terms.size(); i++)
        merged_preset = merge_query(merged_preset, terms.at(i), false);

    merged_preset = merge_query(merged_preset, group_terms, true);

    merged_preset = apply_livelinks(merged_preset, metadata, lookup);

    // spdlog::warn("preprocess_terms\n{}", merged_preset.dump(2));

    auto preprocessed = JsonStore(R"([])"_json);
    preprocessed.push_back(OperatorTermTemplate);
    preprocessed[0]["value"]    = "and";
    preprocessed[0]["children"] = preprocess_terms(merged_preset, entity, query, true, true);

    // spdlog::warn("preprocess_terms\n{}", preprocessed.dump(2));

    try {
        query["query"] = terms_to_query(preprocessed, project_id, entity, lookup);
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        throw;
    }

    return query;
}


utility::JsonStore QueryEngine::build_query(
    const int project_id,
    const std::string &entity,
    const utility::JsonStore &group_terms,
    const utility::JsonStore &terms,
    const utility::JsonStore &context,
    const utility::JsonStore &lookup) {
    auto query = utility::JsonStore(GetQueryResult);

    query["context"] = context;

    auto merged_preset = merge_query(terms, group_terms);

    // spdlog::warn("preprocess_terms\n{}", merged_preset.dump(2));

    auto preprocessed = JsonStore(R"([])"_json);
    preprocessed.push_back(OperatorTermTemplate);
    preprocessed[0]["value"]    = "and";
    preprocessed[0]["children"] = preprocess_terms(merged_preset, entity, query, true, true);

    // spdlog::warn("preprocess_terms\n{}", preprocessed.dump(2));

    try {
        query["query"] = terms_to_query(preprocessed, project_id, entity, lookup);

    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        throw;
    }

    return query;
}

utility::JsonStore QueryEngine::merge_query(
    const utility::JsonStore &base,
    const utility::JsonStore &override,
    const bool ignore_duplicates) {
    auto result = base;

    // we need to preprocess for Disable Global flags..
    auto disable_globals = std::set<std::string>();

    try {
        for (const auto &i : result) {
            if (i.at("enabled").get<bool>() and i.at("term") == "Disable Global")
                disable_globals.insert(i.at("value").get<std::string>());
        }

        // if term already exists in dst, then don't append.
        if (ignore_duplicates) {
            auto dup = std::set<std::string>();
            for (const auto &i : result)
                if (i.at("enabled").get<bool>())
                    dup.insert(i.at("term").get<std::string>());

            for (const auto &i : override) {
                auto term = i.at("term").get<std::string>();
                if (not dup.count(term) and not disable_globals.count(term))
                    result.push_back(i);
            }
        } else {
            for (const auto &i : override) {
                auto term = i.at("term").get<std::string>();
                if (not disable_globals.count(term))
                    result.push_back(i);
            }
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        throw;
    }

    return result;
}

Text QueryEngine::add_text_value(
    const std::string &filter, const std::string &value, const bool negated) {
    if (starts_with(value, "^") and ends_with(value, "$")) {
        if (negated)
            return Text(filter).is_not(value.substr(0, value.size() - 1).substr(1));

        return Text(filter).is(value.substr(0, value.size() - 1).substr(1));
    } else if (ends_with(value, "$")) {
        return Text(filter).ends_with(value.substr(0, value.size() - 1));
    } else if (starts_with(value, "^")) {
        return Text(filter).starts_with(value.substr(1));
    }
    if (negated)
        return Text(filter).not_contains(value);

    return Text(filter).contains(value);
}

void QueryEngine::add_playlist_term_to_filter(
    const std::string &term,
    const std::string &value,
    const bool negated,
    const int project_id,
    const JsonStore &lookup,
    FilterBy *qry) {
    if (term == "Lookback") {
        if (value == "Today")
            qry->push_back(DateTime("updated_at").in_calendar_day(0));
        else if (value == "1 Day")
            qry->push_back(DateTime("updated_at").in_last(1, Period::DAY));
        else if (value == "3 Days")
            qry->push_back(DateTime("updated_at").in_last(3, Period::DAY));
        else if (value == "7 Days")
            qry->push_back(DateTime("updated_at").in_last(7, Period::DAY));
        else if (value == "20 Days")
            qry->push_back(DateTime("updated_at").in_last(20, Period::DAY));
        else if (value == "30 Days")
            qry->push_back(DateTime("updated_at").in_last(30, Period::DAY));
        else if (value == "30-60 Days") {
            qry->push_back(DateTime("updated_at").not_in_last(30, Period::DAY));
            qry->push_back(DateTime("updated_at").in_last(60, Period::DAY));
        } else if (value == "60-90 Days") {
            qry->push_back(DateTime("updated_at").not_in_last(60, Period::DAY));
            qry->push_back(DateTime("updated_at").in_last(90, Period::DAY));
        } else if (value == "100-150 Days") {
            qry->push_back(DateTime("updated_at").not_in_last(100, Period::DAY));
            qry->push_back(DateTime("updated_at").in_last(150, Period::DAY));
        } else if (value == "Future Only") {
            qry->push_back(DateTime("sg_date_and_time").in_next(30, Period::DAY));
        } else {
            throw XStudioError("Invalid query term " + term + " " + value);
        }
    } else if (term == "Playlist Type") {
        if (negated)
            qry->push_back(Text("sg_type").is_not(value));
        else
            qry->push_back(Text("sg_type").is(value));
    } else if (term == "Has Contents") {
        if (value == "False")
            qry->push_back(Text("versions").is_null());
        else if (value == "True")
            qry->push_back(Text("versions").is_not_null());
        else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Site") {
        if (negated)
            qry->push_back(Text("sg_location").is_not(value));
        else
            qry->push_back(Text("sg_location").is(value));
    } else if (term == "Review Location") {
        if (negated)
            qry->push_back(Text("sg_review_location_1").is_not(value));
        else
            qry->push_back(Text("sg_review_location_1").is(value));
    } else if (term == "Department") {
        if (negated)
            qry->push_back(
                Number("sg_department_unit.Department.id")
                    .is_not(resolve_query_value(term, JsonStore(value), lookup).get<int>()));
        else
            qry->push_back(
                Number("sg_department_unit.Department.id")
                    .is(resolve_query_value(term, JsonStore(value), lookup).get<int>()));
    } else if (term == "Author") {
        qry->push_back(Number("created_by.HumanUser.id")
                           .is(resolve_query_value(term, JsonStore(value), lookup).get<int>()));
    } else if (term == "Filter") {
        qry->push_back(QueryEngine::add_text_value("code", value, negated));
    } else if (term == "Tag") {
        qry->push_back(QueryEngine::add_text_value("tags.Tag.name", value, negated));
    } else if (term == "Has Notes") {
        if (value == "False")
            qry->push_back(Text("notes").is_null());
        else if (value == "True")
            qry->push_back(Text("notes").is_not_null());
        else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Unit") {
        auto tmp  = R"({"type": "CustomEntity24", "id":0})"_json;
        tmp["id"] = resolve_query_value(term, JsonStore(value), project_id, lookup).get<int>();
        if (negated)
            qry->push_back(RelationType("sg_unit2").in({JsonStore(tmp)}));
        else
            qry->push_back(RelationType("sg_unit2").not_in({JsonStore(tmp)}));
    } else {
        spdlog::warn("{} Unhandled Term {}", __PRETTY_FUNCTION__, term);
    }
}

void QueryEngine::add_version_term_to_filter(
    const std::string &term,
    const std::string &value,
    const bool negated,
    const int project_id,
    const JsonStore &lookup,
    FilterBy *qry) {
    if (term == "Lookback") {
        if (value == "Today")
            qry->push_back(DateTime("created_at").in_calendar_day(0));
        else if (value == "1 Day")
            qry->push_back(DateTime("created_at").in_last(1, Period::DAY));
        else if (value == "3 Days")
            qry->push_back(DateTime("created_at").in_last(3, Period::DAY));
        else if (value == "7 Days")
            qry->push_back(DateTime("created_at").in_last(7, Period::DAY));
        else if (value == "20 Days")
            qry->push_back(DateTime("created_at").in_last(20, Period::DAY));
        else if (value == "30 Days")
            qry->push_back(DateTime("created_at").in_last(30, Period::DAY));
        else if (value == "30-60 Days") {
            qry->push_back(DateTime("created_at").not_in_last(30, Period::DAY));
            qry->push_back(DateTime("created_at").in_last(60, Period::DAY));
        } else if (value == "60-90 Days") {
            qry->push_back(DateTime("created_at").not_in_last(60, Period::DAY));
            qry->push_back(DateTime("created_at").in_last(90, Period::DAY));
        } else if (value == "100-150 Days") {
            qry->push_back(DateTime("created_at").not_in_last(100, Period::DAY));
            qry->push_back(DateTime("created_at").in_last(150, Period::DAY));
        } else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Playlist") {
        auto tmp  = R"({"type": "Playlist", "id":0})"_json;
        tmp["id"] = resolve_query_value(term, JsonStore(value), project_id, lookup).get<int>();
        qry->push_back(RelationType("playlists").in({JsonStore(tmp)}));
    } else if (term == "Author") {
        qry->push_back(Number("created_by.HumanUser.id")
                           .is(resolve_query_value(term, JsonStore(value), lookup).get<int>()));
    } else if (term == "Older Version") {
        qry->push_back(Number("sg_dneg_version").less_than(std::stoi(value)));
    } else if (term == "Newer Version") {
        qry->push_back(Number("sg_dneg_version").greater_than(std::stoi(value)));
    } else if (term == "Site") {
        if (negated)
            qry->push_back(Text("sg_location").is_not(value));
        else
            qry->push_back(Text("sg_location").is(value));
    } else if (term == "On Disk") {
        std::string prop = std::string("sg_on_disk_") + value;
        if (negated)
            qry->push_back(Text(prop).is("None"));
        else
            qry->push_back(FilterBy().Or(Text(prop).is("Full"), Text(prop).is("Partial")));
    } else if (term == "Pipeline Step") {
        if (negated) {
            if (value == "None")
                qry->push_back(Text("sg_pipeline_step").is_not_null());
            else
                qry->push_back(Text("sg_pipeline_step").is_not(value));
        } else {
            if (value == "None")
                qry->push_back(Text("sg_pipeline_step").is_null());
            else
                qry->push_back(Text("sg_pipeline_step").is(value));
        }
    } else if (term == "Pipeline Status") {
        if (negated)
            qry->push_back(Text("sg_status_list")
                               .is_not(resolve_query_value(term, JsonStore(value), lookup)
                                           .get<std::string>()));
        else
            qry->push_back(Text("sg_status_list")
                               .is(resolve_query_value(term, JsonStore(value), lookup)
                                       .get<std::string>()));
    } else if (term == "Production Status") {
        if (negated)
            qry->push_back(Text("sg_production_status")
                               .is_not(resolve_query_value(term, JsonStore(value), lookup)
                                           .get<std::string>()));
        else
            qry->push_back(Text("sg_production_status")
                               .is(resolve_query_value(term, JsonStore(value), lookup)
                                       .get<std::string>()));
    } else if (term == "Shot Status") {
        if (negated)
            qry->push_back(Text("entity.Shot.sg_status_list")
                               .is_not(resolve_query_value(term, JsonStore(value), lookup)
                                           .get<std::string>()));
        else
            qry->push_back(Text("entity.Shot.sg_status_list")
                               .is(resolve_query_value(term, JsonStore(value), lookup)
                                       .get<std::string>()));
    } else if (term == "Exclude Shot Status") {
        qry->push_back(
            Text("entity.Shot.sg_status_list")
                .is_not(
                    resolve_query_value(term, JsonStore(value), lookup).get<std::string>()));
    } else if (term == "Latest Version") {
        if (value == "False")
            qry->push_back(Text("sg_latest").is_null());
        else if (value == "True")
            qry->push_back(Text("sg_latest").is("Yes"));
        else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Is Hero") {
        if (value == "False")
            qry->push_back(Checkbox("sg_is_hero").is(false));
        else if (value == "True")
            qry->push_back(Checkbox("sg_is_hero").is(true));
        else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Shot") {
        auto rel  = R"({"type": "Shot", "id":0})"_json;
        rel["id"] = resolve_query_value(term, JsonStore(value), project_id, lookup).get<int>();
        qry->push_back(RelationType("entity").is(JsonStore(rel)));
    } else if (term == "Sequence") {
        try {
            auto seq = R"({"type": "Sequence", "id":0})"_json;
            seq["id"] =
                resolve_query_value(term, JsonStore(value), project_id, lookup).get<int>();
            qry->push_back(RelationType("entity").in(std::vector<JsonStore>({JsonStore(seq)})));
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            throw XStudioError("Invalid query term " + term + " " + value);
        }
    } else if (term == "Sent To Client") {
        if (value == "False")
            qry->push_back(DateTime("sg_date_submitted_to_client").is_null());
        else if (value == "True")
            qry->push_back(DateTime("sg_date_submitted_to_client").is_not_null());
        else
            throw XStudioError("Invalid query term " + term + " " + value);


    } else if (term == "Sent To Dailies") {
        if (value == "False")
            qry->push_back(FilterBy().And(
                DateTime("sg_submit_dailies").is_null(),
                DateTime("sg_submit_dailies_chn").is_null(),
                DateTime("sg_submit_dailies_mtl").is_null(),
                DateTime("sg_submit_dailies_van").is_null(),
                DateTime("sg_submit_dailies_mum").is_null()));
        else if (value == "True")
            qry->push_back(FilterBy().Or(
                DateTime("sg_submit_dailies").is_not_null(),
                DateTime("sg_submit_dailies_chn").is_not_null(),
                DateTime("sg_submit_dailies_mtl").is_not_null(),
                DateTime("sg_submit_dailies_van").is_not_null(),
                DateTime("sg_submit_dailies_mum").is_not_null()));
        else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Has Notes") {
        if (value == "False")
            qry->push_back(Text("notes").is_null());
        else if (value == "True")
            qry->push_back(Text("notes").is_not_null());
        else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Filter") {
        qry->push_back(QueryEngine::add_text_value("code", value, negated));
    } else if (term == "Tag") {
        qry->push_back(
            QueryEngine::add_text_value("entity.Shot.tags.Tag.name", value, negated));
    } else if (term == "Reference Tag" or term == "Reference Tags") {

        if (value.find(',') != std::string::npos) {
            // split ...
            for (const auto &i : split(value, ',')) {
                if (negated)
                    qry->push_back(RelationType("tags").name_not_contains(i + ".REFERENCE"));
                else
                    qry->push_back(RelationType("tags").name_is(i + ".REFERENCE"));
            }
        } else {
            if (negated)
                qry->push_back(RelationType("tags").name_not_contains(value + ".REFERENCE"));
            else
                qry->push_back(RelationType("tags").name_is(value + ".REFERENCE"));
        }
    } else if (term == "Tag (Version)") {
        qry->push_back(QueryEngine::add_text_value("tags.Tag.name", value, negated));
    } else if (term == "Twig Name") {
        qry->push_back(QueryEngine::add_text_value("sg_twig_name", value, negated));
    } else if (term == "Twig Type") {
        if (negated)
            qry->push_back(
                Text("sg_twig_type_code")
                    .is_not(resolve_query_value("TwigTypeCode", JsonStore(value), lookup)
                                .get<std::string>()));
        else
            qry->push_back(Text("sg_twig_type_code")
                               .is(resolve_query_value("TwigTypeCode", JsonStore(value), lookup)
                                       .get<std::string>()));
    } else if (term == "Completion Location") {
        auto rel  = R"({"type": "CustomNonProjectEntity16", "id":0})"_json;
        rel["id"] = resolve_query_value(term, JsonStore(value), lookup).get<int>();
        if (negated)
            qry->push_back(
                RelationType("entity.Shot.sg_primary_shot_location").is_not(JsonStore(rel)));
        else
            qry->push_back(
                RelationType("entity.Shot.sg_primary_shot_location").is(JsonStore(rel)));

    } else {
        spdlog::warn("{} Unhandled Term {}", __PRETTY_FUNCTION__, term);
    }
}

void QueryEngine::add_note_term_to_filter(
    const std::string &term,
    const std::string &value,
    const bool negated,
    const int project_id,
    const JsonStore &lookup,
    FilterBy *qry) {
    if (term == "Lookback") {
        if (value == "Today")
            qry->push_back(DateTime("created_at").in_calendar_day(0));
        else if (value == "1 Day")
            qry->push_back(DateTime("created_at").in_last(1, Period::DAY));
        else if (value == "3 Days")
            qry->push_back(DateTime("created_at").in_last(3, Period::DAY));
        else if (value == "7 Days")
            qry->push_back(DateTime("created_at").in_last(7, Period::DAY));
        else if (value == "20 Days")
            qry->push_back(DateTime("created_at").in_last(20, Period::DAY));
        else if (value == "30 Days")
            qry->push_back(DateTime("created_at").in_last(30, Period::DAY));
        else if (value == "30-60 Days") {
            qry->push_back(DateTime("created_at").not_in_last(30, Period::DAY));
            qry->push_back(DateTime("created_at").in_last(60, Period::DAY));
        } else if (value == "60-90 Days") {
            qry->push_back(DateTime("created_at").not_in_last(60, Period::DAY));
            qry->push_back(DateTime("created_at").in_last(90, Period::DAY));
        } else if (value == "100-150 Days") {
            qry->push_back(DateTime("created_at").not_in_last(100, Period::DAY));
            qry->push_back(DateTime("created_at").in_last(150, Period::DAY));
        } else
            throw XStudioError("Invalid query term " + term + " " + value);
    } else if (term == "Filter") {
        qry->push_back(QueryEngine::add_text_value("subject", value, negated));
    } else if (term == "Note Type") {
        if (negated)
            qry->push_back(Text("sg_note_type").is_not(value));
        else
            qry->push_back(Text("sg_note_type").is(value));
    } else if (term == "Author") {
        qry->push_back(Number("created_by.HumanUser.id")
                           .is(resolve_query_value(term, JsonStore(value), lookup).get<int>()));
    } else if (term == "Recipient") {
        auto tmp  = R"({"type": "HumanUser", "id":0})"_json;
        tmp["id"] = resolve_query_value(term, JsonStore(value), lookup).get<int>();
        qry->push_back(RelationType("addressings_to").in({JsonStore(tmp)}));
    } else if (term == "Shot") {
        auto tmp  = R"({"type": "Shot", "id":0})"_json;
        tmp["id"] = resolve_query_value(term, JsonStore(value), project_id, lookup).get<int>();
        qry->push_back(RelationType("note_links").in({JsonStore(tmp)}));
    } else if (term == "Sequence") {
        try {
            auto seq = R"({"type": "Sequence", "id":0})"_json;
            seq["id"] =
                resolve_query_value(term, JsonStore(value), project_id, lookup).get<int>();
            qry->push_back(
                RelationType("note_links").in(std::vector<JsonStore>({JsonStore(seq)})));
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            throw XStudioError("Invalid query term " + term + " " + value);
        }
    } else if (term == "Playlist") {
        auto tmp  = R"({"type": "Playlist", "id":0})"_json;
        tmp["id"] = resolve_query_value(term, JsonStore(value), project_id, lookup).get<int>();
        qry->push_back(RelationType("note_links").in({JsonStore(tmp)}));
    } else if (term == "Version Name") {
        qry->push_back(QueryEngine::add_text_value("note_links.Version.code", value, negated));
    } else if (term == "Tag") {
        qry->push_back(QueryEngine::add_text_value("tags.Tag.name", value, negated));
    } else if (term == "Twig Type") {
        if (negated)
            qry->push_back(
                Text("note_links.Version.sg_twig_type_code")
                    .is_not(resolve_query_value("TwigTypeCode", JsonStore(value), lookup)
                                .get<std::string>()));
        else
            qry->push_back(Text("note_links.Version.sg_twig_type_code")
                               .is(resolve_query_value("TwigTypeCode", JsonStore(value), lookup)
                                       .get<std::string>()));
    } else if (term == "Twig Name") {
        qry->push_back(
            QueryEngine::add_text_value("note_links.Version.sg_twig_name", value, negated));
    } else if (term == "Client Note") {
        if (value == "False")
            qry->push_back(Checkbox("client_note").is(false));
        else if (value == "True")
            qry->push_back(Checkbox("client_note").is(true));
        else
            throw XStudioError("Invalid query term " + term + " " + value);

    } else if (term == "Pipeline Step") {
        if (negated) {
            if (value == "None")
                qry->push_back(Text("sg_pipeline_step").is_not_null());
            else
                qry->push_back(Text("sg_pipeline_step").is_not(value));
        } else {
            if (value == "None")
                qry->push_back(Text("sg_pipeline_step").is_null());
            else
                qry->push_back(Text("sg_pipeline_step").is(value));
        }
    } else if (term == "Older Version") {
        qry->push_back(
            Number("note_links.Version.sg_dneg_version").less_than(std::stoi(value)));
    } else if (term == "Newer Version") {
        qry->push_back(
            Number("note_links.Version.sg_dneg_version").greater_than(std::stoi(value)));
    } else {
        spdlog::warn("{} Unhandled Term {}", __PRETTY_FUNCTION__, term);
    }
}

void QueryEngine::add_term_to_filter(
    const std::string &entity,
    const JsonStore &term,
    const int project_id,
    const JsonStore &lookup,
    FilterBy *qry) {
    auto name    = term.value("term", "");
    auto val     = term.value("value", "");
    auto live    = to_value(term, "livelink", false);
    auto negated = to_value(term, "negated", false);

    // kill queries with invalid shot live link.
    if (val.empty() and live and name == "Shot") {
        auto rel = R"({"type": "Shot", "id":0})"_json;
        qry->push_back(RelationType("entity").is(JsonStore(rel)));
    }

    if (val.empty()) {
        throw XStudioError("Empty query value " + name);
    }

    if (entity == "Playlists")
        add_playlist_term_to_filter(name, val, negated, project_id, lookup, qry);
    else if (entity == "Notes")
        add_note_term_to_filter(name, val, negated, project_id, lookup, qry);
    else if (entity == "Versions")
        add_version_term_to_filter(name, val, negated, project_id, lookup, qry);
    else
        spdlog::warn("{} Unhandled Entity {}", __PRETTY_FUNCTION__, entity);
}


// resolve value from look up
utility::JsonStore QueryEngine::resolve_query_value(
    const std::string &type,
    const utility::JsonStore &value,
    const int project_id,
    const utility::JsonStore &lookup) {
    auto _type        = type;
    auto mapped_value = utility::JsonStore();

    if (_type == "Author" || _type == "Recipient")
        _type = "User";

    if (project_id != -1)
        _type += "-" + std::to_string(project_id);

    try {
        auto val = value.get<std::string>();
        if (lookup.count(_type)) {
            if (lookup.at(_type).count(val)) {
                mapped_value = lookup.at(_type).at(val);
            }
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {} {} {}", _type, __PRETTY_FUNCTION__, err.what(), value.dump(2));
    }

    if (mapped_value.is_null())
        throw XStudioError("Invalid term value " + value.dump());

    return mapped_value;
}

std::string QueryEngine::cache_name(const std::string &type, const int project_id) {
    auto _type = type;
    if (project_id != -1)
        _type += "-" + std::to_string(project_id);
    return _type;
}

std::optional<utility::JsonStore> QueryEngine::get_cache(const std::string &key) const {
    if (cache_.count(key))
        return cache_.at(key);

    return {};
}

void QueryEngine::set_cache(const std::string &key, const utility::JsonStore &data) {
    cache_[key] = data;
}

void QueryEngine::set_shot_sequence_lookup(
    const std::string &key, const utility::JsonStore &data, utility::JsonStore &lookup) {
    auto cache = R"({})"_json;

    try {
        for (const auto &i : data) {
            auto seq = i.at(json::json_pointer("/attributes/code")).get<std::string>();
            for (const auto &s : i.at(json::json_pointer("/relationships/shots/data"))) {
                cache[std::to_string(s.at("id").get<long>())] = seq;
            }
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
    }

    lookup[key] = cache;
}

void QueryEngine::set_lookup(
    const std::string &key, const utility::JsonStore &data, utility::JsonStore &lookup) {
    auto cache = R"({})"_json;

    // load map..
    try {
        for (const auto &i : data) {
            if (i.count("name"))
                cache[i.at("name").get<std::string>()] = i.at("id");
            else if (i.at("attributes").count("name"))
                cache[i.at("attributes").at("name").get<std::string>()] = i.at("id");
            else if (i.at("attributes").count("code"))
                cache[i.at("attributes").at("code").get<std::string>()] = i.at("id");
        }
    } catch (...) {
    }

    // add reverse map
    try {
        for (const auto &i : data) {
            if (i.count("name"))
                cache[i.at("id").get<std::string>()] = i.at("name");
            else if (i.at("attributes").count("name"))
                cache[i.at("id").get<std::string>()] = i.at("attributes").at("name");
            else if (i.at("attributes").count("code"))
                cache[i.at("id").get<std::string>()] = i.at("attributes").at("code");
        }
    } catch (...) {
    }

    lookup[key] = cache;
}


JsonStore QueryEngine::data_from_field(const JsonStore &data) {
    auto result = R"([])"_json;

    std::map<std::string, std::string> entries;

    for (const auto &i : data.at("properties").at("valid_values").at("value")) {
        auto value = i.get<std::string>();
        auto key   = value;
        if (data.at("properties").count("display_values") and
            data.at("properties").at("display_values").at("value").count(value)) {
            key = data.at("properties")
                      .at("display_values")
                      .at("value")
                      .at(value)
                      .get<std::string>();
        }

        entries.insert(std::make_pair(key, value));
    }

    for (const auto &i : entries) {
        auto field                  = R"({"id": null, "attributes": {"name": null}})"_json;
        field["attributes"]["name"] = i.first;
        field["id"]                 = i.second;
        result.push_back(field);
    }

    return JsonStore(result);
}

std::optional<std::string> QueryEngine::get_sequence_name(
    const int project_id, const int shot_id, const utility::JsonStore &lookup) {
    auto key  = cache_name("ShotSequence", project_id);
    auto shot = std::to_string(shot_id);

    if (lookup.count(key)) {
        if (lookup.at(key).count(shot))
            return lookup.at(key).at(shot);
    }

    return {};
}

utility::JsonStore QueryEngine::get_livelink_value(
    const std::string &term,
    const utility::JsonStore &metadata,
    const utility::JsonStore &lookup) {

    auto result = JsonStore();

    try {
        if (metadata.contains(json::json_pointer("/metadata/shotgun/version"))) {
            if (term == "Version Name") {
                result = metadata.at(
                    json::json_pointer("/metadata/shotgun/version/attributes/code"));
            } else if (term == "Older Version" or term == "Newer Version") {
                auto val = metadata
                               .at(json::json_pointer(
                                   "/metadata/shotgun/version/attributes/sg_dneg_version"))
                               .get<long>();
                result = nlohmann::json(std::to_string(val));
            } else if (term == "Author" or term == "Recipient") {
                result = metadata.at(json::json_pointer(
                    "/metadata/shotgun/version/relationships/user/data/name"));
            } else if (term == "Shot") {
                result = metadata.at(json::json_pointer(
                    "/metadata/shotgun/version/relationships/entity/data/name"));
            } else if (term == "Twig Name") {
                result = nlohmann::json(
                    std::string("^") +
                    metadata
                        .at(json::json_pointer(
                            "/metadata/shotgun/version/attributes/sg_twig_name"))
                        .get<std::string>() +
                    std::string("$"));
            } else if (term == "Pipeline Step") {
                result = metadata.at(json::json_pointer(
                    "/metadata/shotgun/version/attributes/sg_pipeline_step"));
            } else if (term == "Twig Type") {
                result = metadata.at(
                    json::json_pointer("/metadata/shotgun/version/attributes/sg_twig_type"));
            } else if (term == "Sequence") {
                auto type = metadata.at(json::json_pointer(
                    "/metadata/shotgun/version/relationships/entity/data/type"));
                if (type == "Sequence") {
                    result = metadata.at(json::json_pointer(
                        "/metadata/shotgun/version/relationships/entity/data/name"));
                } else {
                    auto project_id =
                        metadata
                            .at(json::json_pointer(
                                "/metadata/shotgun/version/relationships/project/data/id"))
                            .get<int>();
                    auto shot_id =
                        metadata
                            .at(json::json_pointer(
                                "/metadata/shotgun/version/relationships/entity/data/id"))
                            .get<int>();

                    auto seq_name = get_sequence_name(project_id, shot_id, lookup);
                    if (seq_name)
                        result = nlohmann::json(*seq_name);
                }
            }
        }
    } catch (const std::exception &err) {
        spdlog::warn("{} {} {} {}", __PRETTY_FUNCTION__, err.what(), term, metadata.dump(2));
    }


    return result;
}
