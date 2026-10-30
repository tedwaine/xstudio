#include "xstudio/utility/data_tree.hpp"

using namespace xstudio::utility;

namespace {
    
    inline std::string to_string2(const xstudio::utility::AnyPtr & ptr) {

        if (!ptr)
            return "null";
        const auto &a = *ptr;
        if (a.type() == typeid(std::string))
            return std::any_cast<const std::string &>(a);
        if (a.type() == typeid(int))
            return std::to_string(std::any_cast<int>(a));
        if (a.type() == typeid(bool))
            return std::any_cast<bool>(a) ? "true" : "false";
        if (a.type() == typeid(float))
            return std::to_string(std::any_cast<float>(a));
        if (a.type() == typeid(JsonStore))
            return std::any_cast<const JsonStore &>(a).dump();
        return "unsupported type";

    }

}

bool PropertyMap::set_data_value_check(const int key, const int64_t &value) {
    AnyPtr current_value = get_data_value(key);
    if (!current_value) {
        set_data_value(key, std::make_shared<const std::any>(value));
        return true;
    }
    const double dv = static_cast<double>(value);
    return set_data_value_check(key, dv);
}

bool PropertyMap::set_data_value_check(const int key, const int &value) {
    AnyPtr current_value = get_data_value(key);
    if (!current_value) {
        set_data_value(key, std::make_shared<const std::any>(value));
        return true;
    }
    const double dv = static_cast<double>(value);
    return set_data_value_check(key, dv);
}

bool PropertyMap::set_data_value_check(const int key, const float &value) {
    AnyPtr current_value = get_data_value(key);
    if (!current_value) {
        set_data_value(key, std::make_shared<const std::any>(value));
        return true;
    }
    const double dv = static_cast<double>(value);
    return set_data_value_check(key, dv);
}

bool PropertyMap::set_data_value_check(const int key, const utility::JsonStore &value) {
    return set_data_value_check(key, static_cast<const nlohmann::json &>(value));
}

bool PropertyMap::set_data_value_check(const int key, const double &value) {
    bool rt = false;
    AnyPtr current_value = get_data_value(key);
    const std::type_info &id = current_value ? current_value->type() : typeid(void);
    if (id == typeid(int64_t)) {
        if (std::any_cast<int64_t>(*(current_value)) != static_cast<int64_t>(value)) {
            set_data_value(key, std::make_shared<const std::any>(static_cast<int64_t>(value)));
            rt = true;
        }
    } else if (id == typeid(int)) {
        if (std::any_cast<int>(*(current_value)) != static_cast<int>(value)) {
            set_data_value(key, std::make_shared<const std::any>(static_cast<int>(value)));
            rt = true;
        }
    } else if (id == typeid(float)) {
        if (std::any_cast<float>(*(current_value)) != static_cast<float>(value)) {
            set_data_value(key, std::make_shared<const std::any>(static_cast<float>(value)));
            rt = true;
        }
    } else if (id == typeid(double)) {
        if (std::any_cast<double>(*(current_value)) != value) {
            set_data_value(key, std::make_shared<const std::any>(value));
            rt = true;
        }
    } else if (id == typeid(void)) {
        set_data_value(key, std::make_shared<const std::any>(value));
        rt = true;
    } else {
        throw std::runtime_error("Unsupported type for set_data_value_check with double");
    }
    return rt; // Value changed
}

bool PropertyMap::set_data_value_check(const int key, const nlohmann::json &data) {

    bool rt = false;
    AnyPtr current_value = get_data_value(key);
    const std::type_info &id = current_value ? current_value->type() : typeid(void);
    if (data.is_string()) {

        if (typeid(utility::Uuid) == id) {
            rt = set_data_value_check(key, utility::Uuid(data.get<std::string>()));
        } else {
            rt = set_data_value_check(key, data.get<std::string>());
        }

    } else if (
        data.is_array() && data.size() == 5 && data.begin().value().is_string() &&
        data.begin().value().get<std::string>() == "vec3") {

        rt = set_data_value_check(key, data.get<Imath::V3f>());

    } else if (
        data.is_array() && data.size() == 6 && data.begin().value().is_string() &&
        data.begin().value().get<std::string>() == "vec4") {

        rt = set_data_value_check(key, data.get<Imath::V4f>());

    } else if (
        data.is_array() && data.size() == 5 && data.begin().value().is_string() &&
        data.begin().value().get<std::string>() == "colour") {

        rt = set_data_value_check(key, data.get<utility::ColourTriplet>());

    } else if (
        (data.is_array() || data.is_object()) &&
        (id == typeid(nlohmann::json) || id == typeid(void))) {

        if (id == typeid(void) || get_data_value_t<nlohmann::json>(key) != data) {
            set_data_value(key, std::make_shared<const std::any>(data));
            rt    = true;
        }

    } else if (data.is_array() && data.size() && data.begin().value().is_string()) {

        std::vector<std::string> v;
        for (auto p = data.begin(); p != data.end(); p++) {
            if (p.value().is_string()) {
                v.push_back(p.value().get<std::string>());
            }
        }
        rt = set_data_value_check(key, v);

    } else if (data.is_array() && data.size() && data.begin().value().is_boolean()) {

        std::vector<bool> v;
        for (auto p = data.begin(); p != data.end(); p++) {
            if (p.value().is_boolean()) {
                v.push_back(p.value().get<bool>());
            }
        }
        rt = set_data_value_check(key, v);

    } else if (data.is_array() && data.size() && data.begin().value().is_number()) {

        std::vector<float> v;
        for (auto p = data.begin(); p != data.end(); p++) {
            if (p.value().is_number()) {
                v.push_back(p.value().get<float>());
            }
        }
        rt = set_data_value_check(key, v);

    } else if (data.is_boolean()) {
        rt = set_data_value_check(key, data.get<bool>());
    } else if (data.is_number_integer()) {
        rt = set_data_value_check(key, data.get<int64_t>());
    } else if (data.is_number_float()) {
        rt = set_data_value_check(key, data.get<float>());
    } else if (id == typeid(nlohmann::json) && std::any_cast<nlohmann::json>(*current_value) != data) {
        set_data_value(key, data);
        rt = true;
    } else if (id == typeid(void)) {
        set_data_value(key, data);
        rt    = true;
    } else if (data.is_null()) {
        rt = true;
    } else {
        std::stringstream msg;
        msg << "Attribute role data is type " << id.name()
            << " doesn't match incoming data.";
        throw std::runtime_error(msg.str().c_str());
    }
    return rt;
}

bool ObjectDataTree::values_equal(const std::any &a, const std::any &b) {
    if (a.type() != b.type())
        return false;
    if (a.type() == typeid(std::string))
        return std::any_cast<const std::string &>(a) == std::any_cast<const std::string &>(b);
    if (a.type() == typeid(int))
        return std::any_cast<int>(a) == std::any_cast<int>(b);
    if (a.type() == typeid(bool))
        return std::any_cast<bool>(a) == std::any_cast<bool>(b);
    if (a.type() == typeid(float))
        return std::any_cast<float>(a) == std::any_cast<float>(b);
    if (a.type() == typeid(JsonStore))
        return std::any_cast<const JsonStore &>(a) == std::any_cast<const JsonStore &>(b);
    return false;
}

bool ObjectDataTree::values_equal(const AnyPtr &a, const AnyPtr &b) {
    if (!a || !b)
        return a == b;
    return values_equal(*a, *b);
}

void ObjectDataTree::dump(const int depth, const int indent) const
{
    for (const auto &prop : data_) {
        spdlog::info("{}{}: {}", std::string(indent, ' '), prop.first, to_string2(prop.second));
    }
    if (depth == 0)
        return;
    spdlog::info("{}Children:", std::string(indent, ' '));
    for (const auto &child : children_data_) {
        child->dump(depth == -1 ? -1 : depth - 1, indent + 2);
    }
}
