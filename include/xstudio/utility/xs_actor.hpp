// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <caf/all.hpp>
#include "xstudio/atoms.hpp"

namespace xstudio {
namespace utility {

class ObjectDataTree;    
using ObjectDataTreePtr = std::shared_ptr<const ObjectDataTree>;

class ObjectDataTree {
public:

    using AnyPtr = std::shared_ptr<const std::any>;
    using PropertyMap = std::map<std::string, AnyPtr>;

    ObjectDataTree()          = default;
    virtual ~ObjectDataTree() = default;

    ObjectDataTreePtr deep_copy() const;

    PropertyMap properties_;
    std::vector<ObjectDataTreePtr> children_data_;
};

/* This base actor class provides a heirarchical structure for actors within
the xSTUDIO session data model. In particular it provides a framework for 
exposing dynamic properties in the UI (Qt/QML) layer.O

ObjectActors can have children (also derived ObjectActor). 
Changes to children are propagated up the tree accordingly. */

class ObjectActor : public caf::event_based_actor {
public:

    ObjectActor(caf::actor_config &cfg);

    virtual ~ObjectActor() = default;

    caf::behavior make_behavior() override {
        return private_message_handler().or_else(message_handler().or_else(extra_message_handlers()));
    }

protected:

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
        mail(property_atom_v, caf::actor_cast<caf::actor_addr>(this), key, full_state_.properties_[key]).send(properties_events_group_);
    }

    void add_child(caf::actor child);
    void remove_child(caf::actor child);

    std::vector<caf::actor_addr> children_;

private:

    caf::message_handler private_message_handler();

    caf::actor properties_events_group_;
    ObjectDataTree full_state_;

};

} // namespace utility
} // namespace xstudio