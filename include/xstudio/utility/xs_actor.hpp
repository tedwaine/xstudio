// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <caf/all.hpp>
#include "xstudio/atoms.hpp"
#include "xstudio/utility/uuid.hpp"

/* Use these macros to register property names for ObjectDataTree.

Any 'property' that you wish to set on an ObjectActor must be registered using these macros.
Provide the name of the model for which you are registering property names as a parameter
of the BEGIN_ macro statement

Use as follows:

BEGIN_REGISTER_PROPERTY_NAMES("SessionModel")
    "nameRole",
    "actorAddressRole",
    ...
END_REGISTER_PROPERTY_NAMES()
*/
#define BEGIN_REGISTER_PROPERTY_NAMES(MODEL_NAME) \
namespace { \
struct ObjectDataTreeRegistrar { \
    const std::string model_name_ = #MODEL_NAME; \
    ObjectDataTreeRegistrar() { \
        std::vector<std::string> roleNames({

#define END_REGISTER_PROPERTY_NAMES() \
        }); \
        xstudio::utility::ObjectDataTree::register_property_names(model_name_, roleNames); \
    } \
} __role_names_registrar; \
} // anonymous namespace

namespace xstudio {
namespace utility {

class ObjectDataTree;    
using ObjectDataTreePtr = std::shared_ptr<const ObjectDataTree>;

class ObjectDataTree {
public:

    static void register_property_names(const std::string &model_name, const std::vector<std::string> &roleNames);
    static const std::map<int, std::string> &property_names(const std::string &model_name);

    using AnyPtr = std::shared_ptr<const std::any>;
    using PropertyMap = std::map<std::string, AnyPtr>;

    // Compares the typed value held by two property values. Supports the
    // property value types used by ObjectDataTree: std::string, int, bool,
    // float and JsonStore. Returns false for unsupported or mismatched types.
    static bool values_equal(const std::any &a, const std::any &b);
    static bool values_equal(const AnyPtr &a, const AnyPtr &b);    

    ObjectDataTree(ObjectDataTree *parent = nullptr) : parent_(parent) {}
    virtual ~ObjectDataTree() = default;

    ObjectDataTree * deep_copy(ObjectDataTree *parent = nullptr) const;

    [[nodiscard]] AnyPtr get_property(const std::string &key) const {
        auto it = properties_.find(key);
        return it != properties_.end() ? it->second : nullptr;
    }

    template <typename T>
    std::optional<T> get_property(const std::string &key) const {
        if (properties_.count(key)) {
            try {
                return std::any_cast<T>(*(properties_.at(key)));
            } catch (const std::bad_any_cast &) {
                return std::nullopt; // Type mismatch
            }
        }
        return std::nullopt; // Property not found
    }

    void dump(const int indent = 0) const;

    PropertyMap properties_;
    std::vector<ObjectDataTreePtr> children_data_;
    ObjectDataTree * parent_ = nullptr;

    private:
    static std::set<std::string> &property_names_mutable(const std::string &model_name);
};

/* This base actor class provides a heirarchical structure for actors within
the xSTUDIO session data model. In particular it provides a framework for 
exposing dynamic properties in the UI (Qt/QML) layer.

An ObjectActor can have any number of children, which are also ObjectActors.
The children are managed in an ordered list. When a child is added, removed 
or relocated in the list the change is propagated up the tree accordingly.

An ObjectActor can have any number of properties. A property is simply a data
value held in a std::any container with an associated key for identification.
When a property is updated, the change is propagated up the tree of ObjectActors
allowing us to automatically update the UI and other dependent components.

*/

class ObjectActor : public caf::event_based_actor {
public:

    ObjectActor(caf::actor_config &cfg, const std::string &name="", const utility::Uuid &uuid = utility::Uuid());

    virtual ~ObjectActor() = default;

    caf::behavior make_behavior() override {
        return private_message_handler().or_else(message_handler().or_else(extra_message_handlers()));
    }

protected:

    friend class Container;

    /** Override this to define the core message handler for your actor implementation. */
    virtual caf::message_handler message_handler() { return caf::message_handler{}; }

    /** Override this to define additional message handlers - for example message handlers from additional classes
    in the inheritance chain between your class and the ObjectActor root base class. */
    virtual caf::message_handler extra_message_handlers() {
        return caf::message_handler{};
    }

    template <typename T>
    void set_property(const std::string &key, const T & value) {
        full_state_.properties_[key] = std::make_shared<const std::any>(value);
        mail(property_atom_v, std::vector<int>(), key, full_state_.properties_[key]).send(properties_events_group_);
    }

    template <typename T>
    std::optional<T> get_property(const std::string &key) const {
        return full_state_.get_property<T>(key);
    }

    AnyPtr get_property(const std::string &key) const {
        return full_state_.get_property(key); // Property not found
    }


    void add_child(const UuidActor &child, const utility::Uuid &before = utility::Uuid());
    void remove_child(const UuidActor &child);
    int child_index(const UuidActor &child) const;
    int child_index(const caf::strong_actor_ptr &child) const;

    std::vector<UuidActor> children_;

private:

    caf::message_handler private_message_handler();

    caf::actor properties_events_group_;
    ObjectDataTree full_state_;

};

} // namespace utility
} // namespace xstudio