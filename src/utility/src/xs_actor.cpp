#include "xstudio/broadcast/broadcast_actor.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/xs_actor.hpp"

using namespace xstudio::utility;

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

ObjectDataTree* ObjectDataTree::deep_copy(ObjectDataTree *parent) const {
    auto copy = new ObjectDataTree(parent);
    for (const auto &child : children_data_) {
        copy->children_data_.push_back(ObjectDataTreePtr(child->deep_copy(copy)));
    }
    for (const auto &prop : properties_) {
        copy->properties_[prop.first] = std::make_shared<const std::any>(*prop.second);
    }
    return copy;
}

void ObjectDataTree::dump(const int depth, const int indent) const
{
    for (const auto &prop : properties_) {
        spdlog::info("{}{}: {}", std::string(indent, ' '), prop.first, to_string2(prop.second));
    }
    if (depth == 0)
        return;
    spdlog::info("{}Children:", std::string(indent, ' '));
    for (const auto &child : children_data_) {
        child->dump(depth == -1 ? -1 : depth - 1, indent + 2);
    }
}

ObjectActor::ObjectActor(caf::actor_config &cfg, const std::string &name, const Uuid &uuid) : caf::event_based_actor(cfg) {
    properties_events_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(properties_events_group_);

    // Note - derived classes that are containers, for example, will
    // set the name and uuid in their own constructors therefore 
    // overwriting these values
    set_property(session::nameRole, name);
    set_property(session::uuidRole, uuid);
    set_property(session::actorRole, caf::actor_cast<caf::actor_addr>(this));

}

void ObjectActor::add_child(const UuidActor &child, const Uuid & before) {

    // monitor the child
    monitor(child.actor(), [this, a = child](const error &err) {
        remove_child(a);
        });

    // add the child to the list of children
    auto it = std::find_if(children_.begin(), children_.end(), [before](const UuidActor &c) {
        return c.uuid() == before;
    });
    children_.insert(it, child);

    // join the child'd property events broadcast group
    mail(property_atom_v, broadcast::join_broadcast_atom_v, caf::actor_cast<caf::actor>(this)).request(child.actor(), infinite).then(
        [=](const bool) {
            // handle response if needed
        },
        [=](const caf::error &err) {
            spdlog::error("{} Error while joining broadcast: {}", __PRETTY_FUNCTION__, to_string(err));
        }
    );

    // fetch the full state of the child's data and re-broadcast it up the tree to our
    // parent (if we have one)
    mail(property_atom_v).request(child.actor(), infinite).then(
        [=](const ObjectDataTreePtr &child_data) mutable {
            //full_state_.children_data_[cidx] = child_data;
            std::vector<int> cidxs{child_index(child)};
            mail(property_atom_v, cidxs, child_data).send(properties_events_group_);
        },
        [=](const caf::error &err) {
            spdlog::error("{} Error while requesting child data: {}", __PRETTY_FUNCTION__, to_string(err));
        }
    );

    /*int cidx = children_.empty() ? 0 : children_.rbegin()->first + 1;
    children_[cidx] = caf::actor_cast<caf::actor_addr>(child);
    mail(property_atom_v).request(child, infinite).then(
        [=](const ObjectDataTreePtr &child_data) mutable {
            //full_state_.children_data_[cidx] = child_data;
            std::vector<int> child_index{cidx};
            mail(property_atom_v, child_index, child_data).send(properties_events_group_);
        },
        [=](const caf::error &err) {
            spdlog::error("{} Error while requesting child data: {}", __PRETTY_FUNCTION__, to_string(err));
        }
    );*/

}

void ObjectActor::child_moved(const Uuid &child_id, const Uuid &before_child_id) {
    
    int cidx = -1;
    for (size_t i = 0; i < children_.size(); ++i) {
        if (children_[i].uuid() == child_id) {
            cidx = static_cast<int>(i);
            break;
        }
    }

    if (cidx == -1) return;

    int before_cidx = -1;    
    for (size_t i = 0; i < children_.size(); ++i) {
        if (children_[i].uuid() == before_child_id) {
            before_cidx = static_cast<int>(i);
            break;
        }
    }

    if ((before_cidx != -1 || before_child_id.is_null()) && cidx != before_cidx) {
        auto to_move = children_[cidx];
        children_.erase(children_.begin() + cidx);
        if (before_cidx == -1) {
            children_.push_back(to_move);
        } else {
            if (cidx < before_cidx) {
                children_.insert(children_.begin() + before_cidx - 1, to_move);
            } else {
                children_.insert(children_.begin() + before_cidx, to_move);
            }
        }
        mail(event_atom_v, move_children_atom_v, std::vector<int>(), cidx, before_cidx == -1 ? (int)children_.size() : before_cidx).send(properties_events_group_);
    }
}

int ObjectActor::child_index(const UuidActor &child) const {
    int cindex = -1;
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        cindex = std::distance(children_.begin(), it);
    }
    return cindex;
}

int ObjectActor::child_index(const caf::strong_actor_ptr &child) const {
    int cindex = -1;
    auto it = std::find_if(children_.begin(), children_.end(), [child](const UuidActor &c) {
        return c.actor() == child;
    });
    if (it != children_.end()) {
        cindex = std::distance(children_.begin(), it);
    }
    return cindex;
}


void ObjectActor::remove_child(const UuidActor &child) {
    int cidx = child_index(child);
    if (cidx != -1)
        children_.erase(children_.begin() + cidx);
    mail(event_atom_v, remove_children_atom_v, std::vector<int>{cidx}).send(properties_events_group_);
    //children_.erase(std::remove(children_.begin(), children_.end(), caf::actor_cast<caf::actor_addr>(child)), children_.end());
}

caf::message_handler ObjectActor::private_message_handler() {

    return caf::message_handler {

        [=](broadcast::broadcast_down_atom, const caf::actor_addr & /*downed_actor*/) {
        },

        [=](move_children_atom, const utility::Uuid &uuid, const utility::Uuid &uuid_before) -> bool {

            child_moved(uuid, uuid_before);
            return true;

        },

        [=](event_atom, move_children_atom, std::vector<int> index_in_tree, const int child_id, const int before_child_id) {

            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(event_atom_v, move_children_atom_v, index_in_tree, child_id, before_child_id).send(properties_events_group_);

        },

        [=](event_atom, remove_children_atom, std::vector<int> index_in_tree) {

            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(event_atom_v, remove_children_atom_v, index_in_tree).send(properties_events_group_);

        },

        [=](find_children_atom, const int prop_key, const AnyPtr &value, int depth) -> result<UuidActor> {

            // try match to ourselves
            auto prop = __get_property(prop_key);

            if (ObjectDataTree::values_equal(prop, value)) {
                auto uuid = get_property<Uuid>(session::uuidRole);
                if (uuid) {
                    return UuidActor(*uuid, caf::actor_cast<caf::actor>(this));
                }
            }

            if (depth <= 0) {
                return caf::make_error(xstudio_error::error, "No matching child found.");
            }

            auto auto_responder = AutoResponder<UuidActor>(children_.size(), this);
            for (auto child : children_) {
                mail(find_children_atom_v, prop_key, value, depth-1).request(child.actor(), infinite).then(
                    [=](const UuidActor &c) mutable {
                        auto_responder.result() = c;
                        auto_responder.decrement();
                    },
                    [=](const caf::error &err) mutable {
                        auto_responder.decrement();
                    });
            }
            
            return auto_responder.response_promise();
        },

        [=](add_children_atom, const UuidActor &child, const Uuid &before) -> bool {
            add_child(child, before);
            return true;
        },
        [=](add_children_atom, const UuidActor &child) -> bool {
            add_child(child);
            return true;
        },
        [=](add_children_atom, const UuidActorVector &childred) -> bool {
            for (const auto &child : childred) {
                add_child(child);
            }
            return true;
        },
        [=](property_atom, const int key, const uint64_t ui_layer_node_id, caf::actor sender) {
            auto r = __get_property(key);
            if (r) {
                mail(utility::property_atom_v, r, key, ui_layer_node_id).send(sender);
            }
        },
        [=](property_atom, broadcast::leave_broadcast_atom, caf::actor subscriber) -> result <bool> {
            auto rp = make_response_promise<bool>();
            mail(broadcast::leave_broadcast_atom_v, subscriber).request(properties_events_group_, infinite).then(
                [=](const bool r) mutable {
                    rp.deliver(r);
                },
                [=](caf::error &err) mutable {
                    rp.deliver(err);
                }
            );
            return rp;
        },
        [=](property_atom, broadcast::join_broadcast_atom, caf::actor subscriber) -> result <bool> {
            auto rp = make_response_promise<bool>();
            mail(broadcast::join_broadcast_atom_v, subscriber).request(properties_events_group_, infinite).then(
                [=](const bool r) mutable {
                    rp.deliver(r);
                },
                [=](caf::error &err) mutable {
                    rp.deliver(err);
                }
            );
            return rp;
        },
        [=](property_atom, const int key, const AnyPtr &value) -> bool {

            full_state_.properties_[key] = value;
            mail(property_atom_v, std::vector<int>(), key, value).send(properties_events_group_);
            return true;
        },
        [=](property_atom, std::vector<int> index_in_tree, const int key, const AnyPtr &value) {
            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index(current_sender());
             std::cerr << "donk " << *(get_property<std::string>(session::nameRole)) << " " << key << std::endl;
            if (cindex < 0) {
                std::cerr << "nope " << *(get_property<std::string>(session::nameRole)) << std::endl;
                return;
            }
            // bubble it up
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(property_atom_v, index_in_tree, key, value).send(properties_events_group_);
        },
        [=](property_atom, const ObjectDataTreePtr &child_data) {

            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index(current_sender());
            if (cindex < 0) return;

            // bubble it up
            mail(property_atom_v, std::vector<int>{cindex}, child_data).send(properties_events_group_);

        },
        [=](property_atom, std::vector<int> index_in_tree, const ObjectDataTreePtr &child_data) {

            // A child's data (or the child of a child etc.) has been updated. Foward the update
            // to our parent via properties_events_group_. We append the index of the child the
            // forwarded this to us to index_in_tree.
            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(property_atom_v, index_in_tree, child_data).send(properties_events_group_);

        },
        [=](property_atom, const int key, const AnyPtr &value) {

            // A property has been updated.
            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            mail(property_atom_v, std::vector<int>{cindex}, key, value).send(properties_events_group_);

        },
        [=](property_atom, const int key) -> result<AnyPtr> {
            return full_state_.properties_.count(key) ? full_state_.properties_[key] : AnyPtr();
        },
        [=](property_atom) -> result<ObjectDataTreePtr> {

            auto auto_responder = AutoResponder<ObjectDataTreePtr>(children_.size(), this);
            auto result = full_state_.deep_copy();            
            auto_responder.result().reset(result); 
            for (size_t i = 0; i < children_.size(); ++i) {
                mail(property_atom_v).request(children_[i].actor(), infinite).then(
                    [=](ObjectDataTreePtr cdata) mutable {
                        // this const_cast is ok as we 'own' cdata - it was a copy
                        // made by the child and returned to us by this very
                        // message handler (in the child)
                        const_cast<ObjectDataTree *>(cdata.get())->parent_ = result;
                        result->children_data_.resize(std::max(result->children_data_.size(), size_t(i+1)));
                        result->children_data_[i] = cdata;
                        auto_responder.decrement();
                    },
                    [=](const caf::error &err) mutable{
                        auto_responder.decrement(err);
                    }
                );
            }
            return auto_responder.response_promise();
        }
    };
}
    