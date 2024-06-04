// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <nlohmann/json.hpp>

#include "xstudio/utility/json_store.hpp"
#include "xstudio/utility/json_store_sync.hpp"
#include "xstudio/shotgun_client/shotgun_client.hpp"
#include "data_source_shotgun_definitions.hpp"

using namespace xstudio;

const auto OperatorTermTemplate = R"({
    "id": null,
    "type": "term",
    "term": "operator",
    "value": "and/or",
    "negated": false,
    "enabled": true,
    "children": []
})"_json;

// "dynamic": false,
const auto TermTemplate = R"({
	"id": null,
	"type": "term",
    "term": "",
    "value": "",
    "enabled": true,
    "livelink": null,
    "negated": null
})"_json;

const auto PresetTemplate = R"({
	"id": null,
	"type": "preset",
	"name": "PRESET",
    "hidden": false,
    "update": null,
    "children": []
})"_json;

const auto GroupTemplate = R"({
	"id": null,
	"type": "group",
	"name": "GROUP",
    "hidden": false,
	"entity": "",
    "children": [
        {
            "id": null,
            "type": "preset",
            "name": "PRESET",
            "update": false,
            "children": []
        },
    	{
    		"id": null,
    		"type": "presets",
    		"children": []
    	}
    ]
})"_json;

const auto RootTemplate = R"({
	"id": null,
	"name": "root",
	"type": "root",
    "children": []
})"_json;

class QueryEngine {
  public:
    QueryEngine() {
        set_lookup(
            cache_name("Completion Location"), utility::JsonStore(locationsJSON), lookup_);
        set_lookup(cache_name("Twig Type"), TwigTypeCodes, lookup_);
        initialise_presets();
    }
    virtual ~QueryEngine() = default;

    static utility::JsonStore build_query(
        const int project_id,
        const std::string &entity,
        const utility::JsonStore &group_terms,
        const utility::JsonStore &terms,
        const utility::JsonStore &context,
        const utility::JsonStore &lookup);

    static utility::JsonStore build_query_new(
        const int project_id,
        const std::string &entity,
        const utility::JsonStore &group_terms,
        const utility::JsonStore &terms,
        const utility::JsonStore &context,
        const utility::JsonStore &metadata,
        const utility::JsonStore &lookup);

    static utility::JsonStore merge_query(
        const utility::JsonStore &base,
        const utility::JsonStore &override,
        const bool ignore_duplicates = true);

    static shotgun_client::Text add_text_value(
        const std::string &filter, const std::string &value, const bool negated = false);

    static void add_term_to_filter(
        const std::string &entity,
        const utility::JsonStore &term,
        const int project_id,
        const utility::JsonStore &lookup,
        shotgun_client::FilterBy *qry);

    static utility::JsonStore resolve_query_value(
        const std::string &type,
        const utility::JsonStore &value,
        const utility::JsonStore &lookup) {
        return resolve_query_value(type, value, -1, lookup);
    }

    static utility::JsonStore resolve_query_value(
        const std::string &type,
        const utility::JsonStore &value,
        const int project_id,
        const utility::JsonStore &lookup);

    static std::string cache_name(const std::string &type, const int project_id = -1);
    static utility::JsonStore data_from_field(const utility::JsonStore &data);

    static std::optional<std::string> get_sequence_name(
        const int project_id, const int shot_id, const utility::JsonStore &lookup);

    static utility::JsonStore get_livelink_value(
        const std::string &term,
        const utility::JsonStore &metadata,
        const utility::JsonStore &lookup);

    static void set_lookup(
        const std::string &key, const utility::JsonStore &data, utility::JsonStore &lookup);
    static void set_shot_sequence_lookup(
        const std::string &key, const utility::JsonStore &data, utility::JsonStore &lookup);

    std::optional<utility::JsonStore> get_cache(const std::string &key) const;
    void set_cache(const std::string &key, const utility::JsonStore &data);

    utility::JsonStore &lookup() { return lookup_; }
    utility::JsonStoreSync &user_presets() { return user_presets_; }
    utility::JsonStore &system_presets() { return system_presets_; }

    void initialise_presets();
    void set_presets(const utility::JsonStore &user, const utility::JsonStore &system);
    void merge_presets(nlohmann::json &user_presets, const nlohmann::json &system_presets);

    const utility::Uuid &uuid() const { return uuid_; }

    template <typename T>
    static T to_value(const nlohmann::json &jsn, const std::string &key, const T &fallback);

    static bool precache_needed(const int project_id, const utility::JsonStore &lookup);

    static utility::JsonStore apply_livelinks(
        const utility::JsonStore &terms,
        const utility::JsonStore &metadata,
        const utility::JsonStore &lookup);

  private:
    void merge_group(nlohmann::json &user_presets, const nlohmann::json &system_group);

    bool preset_diff(const nlohmann::json &a, const nlohmann::json &b);

    // handle expansion into OR/AND
    static utility::JsonStore preprocess_terms(
        const utility::JsonStore &terms,
        const std::string &entity,
        utility::JsonStore &query,
        const bool and_mode = true,
        const bool initial  = true);

    static shotgun_client::FilterBy terms_to_query(
        const utility::JsonStore &terms,
        const int project_id,
        const std::string &entity,
        const utility::JsonStore &lookup,
        const bool and_mode = true,
        const bool initial  = true);

    static void add_playlist_term_to_filter(
        const std::string &term,
        const std::string &value,
        const bool negated,
        const int project_id,
        const utility::JsonStore &lookup,
        shotgun_client::FilterBy *qry);

    static void add_version_term_to_filter(
        const std::string &term,
        const std::string &value,
        const bool negated,
        const int project_id,
        const utility::JsonStore &lookup,
        shotgun_client::FilterBy *qry);

    static void add_note_term_to_filter(
        const std::string &term,
        const std::string &value,
        const bool negated,
        const int project_id,
        const utility::JsonStore &lookup,
        shotgun_client::FilterBy *qry);

    utility::JsonStoreSync user_presets_;
    utility::JsonStore system_presets_;

    utility::JsonStore cache_;
    utility::JsonStore lookup_;

    utility::Uuid uuid_ = utility::Uuid::generate();
};
