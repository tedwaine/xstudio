// SPDX-License-Identifier: Apache-2.0
#include "xstudio/atoms.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/module/attribute.hpp"
#include "xstudio/module/module.hpp"

#include <iostream>
#include <typeinfo>
#include <typeindex>

using namespace xstudio::module;
using namespace xstudio;

namespace {

template <class T, class F>
inline std::pair<const std::type_index, std::function<nlohmann::json(utility::AnyPtr const &)>>
to_any_to_json(F const &f) {
    return {std::type_index(typeid(T)), [g = f](utility::AnyPtr const &a) -> nlohmann::json {
                return g(std::any_cast<T const &>(*a));
            }};
}

/*
Map of functions to convert certain types wrapped by std::any to nlohmann::json
*/

inline const std::
    unordered_map<std::type_index, std::function<nlohmann::json(utility::AnyPtr const &)>> &
    any_to_json() {

    static std::unordered_map<std::type_index, std::function<nlohmann::json(utility::AnyPtr const &)>>
        _any_to_json{
            to_any_to_json<nlohmann::json>(
                [](nlohmann::json x) -> nlohmann::json { return x; }),
            to_any_to_json<int>(
                [](int x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<int64_t>(
                [](int64_t x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<float>([](float x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<double>(
                [](double x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<bool>([](bool x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<utility::Uuid>(
                [](utility::Uuid x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<std::string>(
                [](std::string x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<std::array<float, 4>>(
                [](std::array<float, 4> x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<Imath::V3f>([](Imath::V3f x) -> nlohmann::json {
                return nlohmann::json{"vec3", 1, x.x, x.y, x.z};
            }),
            to_any_to_json<Imath::V4f>([](Imath::V4f x) -> nlohmann::json {
                return nlohmann::json{"vec4", 1, x.x, x.y, x.z, x.w};
            }),
            to_any_to_json<utility::ColourTriplet>(
                [](utility::ColourTriplet x) -> nlohmann::json {
                    return nlohmann::json{"colour", 1, x.r, x.g, x.b};
                }),
            to_any_to_json<std::vector<float>>(
                [](std::vector<float> x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<std::vector<int>>(
                [](std::vector<int> x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<std::vector<bool>>(
                [](std::vector<bool> x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<std::vector<std::string>>(
                [](std::vector<std::string> x) -> nlohmann::json { return nlohmann::json(x); }),
            to_any_to_json<std::vector<utility::Uuid>>(
                [](std::vector<utility::Uuid> x) -> nlohmann::json {
                    return nlohmann::json(x);
                }),
        };
    return _any_to_json;
}


    nlohmann::json to_json(const utility::AnyPtr &v) {

        if (v && v->has_value()) {
            auto atj = any_to_json();
            if (const auto it = atj.find(std::type_index(v->type())); it != atj.cend()) {
                return it->second(v);
            } else {
                spdlog::warn(
                    "{} Unregistered type {}", __PRETTY_FUNCTION__, v->type().name());
            }
        }

        return {};

    }
} // namespace module

Attribute::Attribute(
    const std::string &title, const std::string &abbr_title, const std::string &type_name) {
    role_data_.set_data_value(Enabled, true);
    role_data_.set_data_value(Type, type_name);
    role_data_.set_data_value(Title, title);
    role_data_.set_data_value(AbbrTitle, abbr_title);
    role_data_.set_data_value(UuidRole, utility::Uuid::generate());
    role_data_.set_data_value(UIDataModels, std::vector<std::string>());
}

void Attribute::set_owner(Module *owner) { owner_ = owner; }

void Attribute::set_redraw_viewport_on_change(const bool redraw_viewport_on_change) {
    redraw_viewport_on_change_ = redraw_viewport_on_change;
}

nlohmann::json Attribute::role_data_as_json(const int role) const {

    const utility::AnyPtr v = role_data_.get_data_value(role);
    if (!v) {
        std::stringstream msg;
        msg << "Attribute::role_data_as_json " << get_role_data<std::string>(Title)
            << " of type " << get_role_data<std::string>(Type)
            << " does not have role data for " << role_name(role) << "\n";
        throw std::runtime_error(msg.str().c_str());
    }
    
    return to_json(v);
}

void Attribute::update_role_data_from_json(
    const int role, const nlohmann::json &data, const bool notify) {

    if (role != UuidRole)
        set_role_data(role, data, notify);
}

void Attribute::notify_change(
    const int role, const nlohmann::json &data, const bool self_notify) {
    if (owner_) {
        owner_->notify_change(
            uuid(), role, role_data_as_json(role), redraw_viewport_on_change_, self_notify);
    }
}

nlohmann::json Attribute::as_json() const {

    nlohmann::json result;
    for (const auto &p : role_data_) {
        result[role_name(p.first)] = to_json(p.second);
    }

    return result;
}

void Attribute::update_from_json(const nlohmann::json &data, const bool notify) {

    if (data.is_object()) {

        for (auto p = data.begin(); p != data.end(); p++) {
            int role = role_index(p.key());
            if (role != UuidRole)
                set_role_data(role, p.value(), notify);
        }
    }
}

bool Attribute::belongs_to_groups(const std::vector<std::string> &group_names) const {

    bool rt                     = false;
    const auto attr_group_names = get_role_data<std::vector<std::string>>(UIDataModels);
    for (const auto &p : attr_group_names) {
        if (std::find(group_names.begin(), group_names.end(), p) != group_names.end()) {
            rt = true;
            break;
        }
    }
    return rt;
}

// Deliberately out-of-line -- see the comment on the declaration in
// attribute.hpp. This must stay in libmodule so that the any_cast inside
// get_role_data<utility::Uuid>() is instantiated exactly once, in the same
// image that set the role data in Attribute's constructor.
utility::Uuid Attribute::uuid() const { return get_role_data<utility::Uuid>(UuidRole); }

std::string Attribute::role_name(const int role) {
    const auto &p = role_names.find(role);
    if (p != role_names.end()) {
        return p->second;
    }
    return std::string("BadRoleID");
}

int Attribute::role_index(const std::string &role_name) {
    int rt = -1;
    for (auto p : role_names) {
        if (p.second == role_name) {
            rt = p.first;
            break;
        }
    }
    return rt;
}

std::string Attribute::type_name(const int tp) {
    const auto &p = type_names.find(tp);
    if (p != type_names.end()) {
        return p->second;
    }
    return std::string("BadRoleID");
}

void Attribute::set_preference_path(const std::string &preference_path) {
    set_role_data(PreferencePath, preference_path);
}

void Attribute::expose_in_ui_attrs_group(const std::string &group_name, bool expose) {
    if (expose) {
        if (!has_role_data(UIDataModels)) {
            set_role_data(UIDataModels, std::vector<std::string>({group_name}));
            return;
        }
        auto n = get_role_data<std::vector<std::string>>(UIDataModels);
        for (const auto &g : n) {
            if (g == group_name)
                return;
        }
        n.push_back(group_name);
        set_role_data(UIDataModels, n);

    } else if (has_role_data(UIDataModels)) {
        auto n = get_role_data<std::vector<std::string>>(UIDataModels);
        for (auto p = n.begin(); p != n.end(); ++p) {
            if (*p == group_name) {
                n.erase(p);
                set_role_data(UIDataModels, n);
                return;
            }
        }
    }
}

void Attribute::set_tool_tip(const std::string &tool_tip) { set_role_data(ToolTip, tool_tip); }
