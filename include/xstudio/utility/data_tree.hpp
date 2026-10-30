// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <caf/all.hpp>
#include "xstudio/atoms.hpp"
#include "xstudio/utility/uuid.hpp"
#include "xstudio/utility/json_store.hpp"

namespace xstudio {
namespace utility {

using AnyPtr = std::shared_ptr<const std::any>;

/**
 * @class PropertyMap
 * @brief A simplified wrapper for a map of typed properties.
 *
 * This class is used to store data values associated with integer keys.
 *
 * Values are stored as shared pointers to constant std::any objects, 
 * allowing for type-safe retrieval and optional presence checking.
 *
 * Generally data is only read when it needs to be converted to 
 * QVariant for use in the Qt/QML layer, for which we have a single
 * conversion function that handles this efficiently.
 *
 */
class PropertyMap : private std::map<int, AnyPtr> {

    public:

    PropertyMap() = default;

    void delete_entry(const int key) {
        auto it = find(key);
        if (it != end()) {
            erase(it);
        }
    }

    AnyPtr get_data_value(const int key) const {
        auto it = find(key);
        return it != end() ? it->second : nullptr;
    }

    bool has_data_value(const int key) const {
        return find(key) != end();
    }

    template <typename T>
    T get_data_value_t(const int key) const {
        auto it = find(key);
        if (it != end() && it->second) {
            try {
                return std::any_cast<T>(*(it->second));
            } catch (std::exception & e) {
                throw std::runtime_error(std::string("Failed to cast property to the requested type: ") + typeid(T).name() + " : " + it->second->type().name());
            }
        } else {
            throw std::runtime_error("Property not found");
        }
        return T{};
    }

    template <typename T>
    std::optional<T> get_data_value(const int key) const {
        auto it = find(key);
        if (it != end() && it->second) {
            try {
                return std::any_cast<T>(*(it->second));
            } catch (const std::bad_any_cast &) {
                return std::nullopt; // Type mismatch
            }
        }
        return std::nullopt; // Property not found
    }

    void set_data_value(const int key, const AnyPtr &value) {
        (*this)[key] = value;
    }

    // specialized version for double - allows setting of underlying
    // int, int64_t, float, and double types
    bool set_data_value_check(const int key, const double &value);

    bool set_data_value_check(const int key, const int64_t &value);

    bool set_data_value_check(const int key, const int &value);

    bool set_data_value_check(const int key, const float &value);

    bool set_data_value_check(const int key, const utility::JsonStore &value);

    bool set_data_value_check(const int key, const nlohmann::json &value);

    template <typename T>
    bool set_data_value_check(const int key, const T &value) {
        auto it = find(key);
        if (it != end() && it->second && it->second->type() == typeid(T)) {
            if (std::any_cast<T>(*(it->second)) == value) {
                return false; // No change
            }
        }
        set_data_value(key, value);
        return true; // Value changed
    }

    template <typename T>
    void set_data_value(const int key, const T &value) {
        (*this)[key] = std::make_shared<const std::any>(value);
    }

    using std::map<int, AnyPtr>::cbegin;
    using std::map<int, AnyPtr>::cend;
    using std::map<int, AnyPtr>::begin;
    using std::map<int, AnyPtr>::end;

};

/**
 * @class ObjectDataTree
 * @brief A tree structure for storing hierarchical data with typed properties.
 *
 * This class is used to mirror actor state in a 'light' hierarchical tree structure
 * that reflects the hierarchical structure of the actor objects that it represents.
 *
 * For example, the actors that make up the xSTUDIO session (session, playlists, 
 * subsets, contact sheets, media, media_sources, media streams) exist in a tree 
 * like structure with the SessionActor at the root. Each of these actor types 
 * inherit from ObjectActor which stores certain data values relating to its state
 * in an ObjectDataTree. Any changes to data are automatically broadcast to the 
 * parent actor, which re-broadcasts the changes to its own parent, and so on up 
 * the hierarchy. Another actor can be set-up to receive these updates to the 
 * tree structure, allowing it to stay in sync with the actor hierarchy and its state.
 *
 */

class ObjectDataTree;    
using ObjectDataTreePtr = std::shared_ptr<const ObjectDataTree>;

class ObjectDataTree {
public:

    // Compares the typed value held by two property values. Supports the
    // property value types used by ObjectDataTree: std::string, int, bool,
    // float and JsonStore. Returns false for unsupported or mismatched types.
    static bool values_equal(const std::any &a, const std::any &b);
    static bool values_equal(const AnyPtr &a, const AnyPtr &b);    

    ObjectDataTree(const PropertyMap &properties = {}) : data_(properties) {}
    virtual ~ObjectDataTree() = default;

    bool has_data_value(const int key) const {
        return data_.has_data_value(key);
    }

    [[nodiscard]] AnyPtr get_data_value(const int key) const { return data_.get_data_value(key); }

    template <typename T>
    std::optional<T> get_data_value(const int key) const { return data_.get_data_value<T>(key); }

    void set_data_value(const int key, const AnyPtr &value) { data_.set_data_value(key, value); }

    template <typename T>
    void set_data_value(const int key, const T &value) { data_.set_data_value<T>(key, value); }

    void dump(const int depth = -1, const int indent = 0) const;

    int num_nodes() const { return static_cast<int>(children_data_.size()); }

    ObjectDataTree * parent() const { return parent_; }

    void set_parent(ObjectDataTree *parent) { parent_ = parent; }

    void new_node(const ObjectDataTreePtr &node, const int index) {
        children_data_.resize(std::max(static_cast<size_t>(index) + 1, children_data_.size()));
        children_data_[index] = node;
    }

    void insert_node(const ObjectDataTreePtr &node, const int index) {
        if (index < 0 || index >= num_nodes()) {
            children_data_.push_back(node);
        } else {
            auto p = children_data_.begin() + index;
            children_data_.insert(p, node);
        }
    }

    void move_node(const int row, const int destination_row) {
        if (row < 0 || row >= num_nodes() || destination_row < 0 || destination_row > num_nodes())
            return;
        auto to_move = children_data_[row];
        children_data_.erase(children_data_.begin() + row);
        if (destination_row > row) {
            children_data_.insert(children_data_.begin() + destination_row - 1, to_move);
        } else {
            children_data_.insert(children_data_.begin() + destination_row, to_move);
        }
    }

    void remove_node(const int row) {
        if (row < 0 || row >= num_nodes())
            return;
        children_data_.erase(children_data_.begin() + row);
    }

    ObjectDataTreePtr get_node(const int row) const {
        if (row < 0 || row >= num_nodes())
            return nullptr;
        return children_data_[row];
    }

    std::vector<ObjectDataTreePtr>::const_iterator begin() const { return children_data_.begin(); }
    std::vector<ObjectDataTreePtr>::const_iterator end() const { return children_data_.end(); }
    std::vector<ObjectDataTreePtr>::const_iterator cbegin() const { return children_data_.cbegin(); }
    std::vector<ObjectDataTreePtr>::const_iterator cend() const { return children_data_.cend(); }

    private:

    PropertyMap data_;
    std::vector<ObjectDataTreePtr> children_data_;
    ObjectDataTree * parent_ = nullptr;

};

} // namespace utility
} // namespace xstudio
