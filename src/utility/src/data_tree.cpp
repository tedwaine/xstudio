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
