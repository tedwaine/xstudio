// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <caf/all.hpp>
#include "xstudio/atoms.hpp"
#include "xstudio/utility/uuid.hpp"
#include "xstudio/utility/data_tree.hpp"

namespace xstudio {
namespace utility {


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

    /** Override this to implement on-demand property retrieval in your actor. This method
    is only called when the property is not found in the local property map. */
    virtual AnyPtr get_data_value(const int key) { return AnyPtr(); }

    /** Override this to define additional message handlers - for example message handlers from additional classes
    in the inheritance chain between your class and the ObjectActor root base class. */
    virtual caf::message_handler extra_message_handlers() {
        return caf::message_handler{};
    }

    template <typename T>
    void set_data_value(const int key, const T & value) {
        data_.set_data_value(key, value);
        mail(event_atom_v, property_atom_v, std::vector<int>(), key, data_.get_data_value(key)).send(properties_events_group_);
    }

    template <typename T>
    std::optional<T> get_data_value(const int key) const {
        return data_.get_data_value<T>(key);
    }

    void add_child(const UuidActor &child, const utility::Uuid &before = utility::Uuid());
    void remove_child(const UuidActor &child);

    void child_moved(const Uuid &child_id, const Uuid &before_child_id);

    int child_index(const UuidActor &child) const;
    int child_index(const caf::strong_actor_ptr &child) const;

    std::vector<UuidActor> children_;

    caf::event_based_actor *self() { return static_cast<caf::event_based_actor *>(this); }

private:

    caf::message_handler private_message_handler();

    AnyPtr __get_property(const int key) {
        AnyPtr result = data_.get_data_value(key);
        if (!result) {
            result = get_data_value(key); // Try to get it on-demand if not found locally
            if (result) {
                data_.set_data_value(key, result);
            }
        }
        return result;
    }

    caf::actor properties_events_group_;
    PropertyMap data_;

};

} // namespace utility
} // namespace xstudio