#include "xstudio/broadcast/broadcast_actor.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/xs_actor.hpp"

using namespace xstudio::utility;


ObjectActor::ObjectActor(caf::actor_config &cfg, const std::string &name, const Uuid &uuid) : caf::event_based_actor(cfg) {
    properties_events_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(properties_events_group_);

    // Note - derived classes that are containers, for example, will
    // set the name and uuid in their own constructors therefore 
    // overwriting these values
    set_data_value(session::nameRole, name);
    set_data_value(session::uuidRole, uuid);
    set_data_value(session::actorRole, caf::actor_cast<caf::actor_addr>(this));

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
    const int insert_index = std::distance(children_.begin(), it);
    children_.insert(it, child);

    // join the child'd property events broadcast group
    self()->mail(property_atom_v, broadcast::join_broadcast_atom_v, caf::actor_cast<caf::actor>(this)).request(child, infinite).then(
        [=](const bool) {
            // handle response if needed
        },
        [=](const caf::error &err) {

            std::cerr << "FAO " << to_string(child.actor()) << "\n";
            self()->mail(name_atom_v).request(child.actor(), infinite).then(
                [=](const std::string &name) {
                    std::cerr << "Child name: " << name << "\n";
                },
                [=](const caf::error &err) {
                    std::cerr << "Error fetching child name: " << to_string(err) << "\n";
                }
            );

            spdlog::error("{} Error while joining broadcast: {}", __PRETTY_FUNCTION__, to_string(err));
        }
    );

    self()->mail(property_atom_v).request(child, infinite).then(
        [=](const ObjectDataTreePtr &child_data) {

            mail(event_atom_v, add_children_atom_v, std::vector<int>{}, insert_index, child_data).send(properties_events_group_);

        },
        [=](const caf::error &err) {
            log_caf_error(err);
        });

}

void ObjectActor::child_moved(const Uuid &child_id, const Uuid &before_child_id) {
    
    // get the index of the child to be moved
    int cidx = -1;
    for (size_t i = 0; i < children_.size(); ++i) {
        if (children_[i].uuid() == child_id) {
            cidx = static_cast<int>(i);
            break;
        }
    }
    if (cidx == -1) return;

    // get the index of the child before which the current child should be moved
    // if before_child_id is null, it means the child should be moved to the end of the list
    int before_cidx = -1;    
    if (!before_child_id.is_null()) {
        for (size_t i = 0; i < children_.size(); ++i) {
            if (children_[i].uuid() == before_child_id) {
                before_cidx = static_cast<int>(i);
                break;
            }
        }
    }

    if ((before_cidx != -1 || before_child_id.is_null()) && cidx != before_cidx) {

        // move the child to the new position in the children_ vector
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
        // notify down the tree that a child has been moved
        mail(
            event_atom_v,
            move_children_atom_v,
            std::vector<int>(),
            cidx,
            before_cidx == -1 ? (int)children_.size() : before_cidx).send(properties_events_group_);
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

            // children were moved further up the tree. Rebroadcast.
            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(event_atom_v, move_children_atom_v, index_in_tree, child_id, before_child_id).send(properties_events_group_);

        },

        [=](event_atom, remove_children_atom, std::vector<int> index_in_tree) {

            // a child was removed further up the tree. Rebroadcast.
            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(event_atom_v, remove_children_atom_v, index_in_tree).send(properties_events_group_);

        },

        [=](event_atom, add_children_atom, std::vector<int> index_in_tree, const int insertion_index, const ObjectDataTreePtr &child_data) {

            // a child was added further up the tree. Rebroadcast.
            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(event_atom_v, add_children_atom_v, index_in_tree, insertion_index, child_data).send(properties_events_group_);

        },

        [=](find_children_atom, const int prop_key, const AnyPtr &value, int depth) -> result<UuidActor> {

            // This allows us to recursively search the tree to find an actor that has
            // a property with the specified key and value.

            // try match to ourselves
            auto prop = __get_property(prop_key);

            if (ObjectDataTree::values_equal(prop, value)) {
                auto uuid = get_data_value<Uuid>(session::uuidRole);
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
        [=](add_children_atom, const UuidActorVector &children) -> bool {
            for (const auto &child : children) {
                add_child(child);
            }
            return true;
        },
        [=](property_atom, const int key, const uint64_t ui_layer_node_id, caf::actor sender) {
            // This is an a-sync request for data from a subscriber to the tree. We respond with the requested property if it exists.
            // Our virtual 'get_data_value' method is called, giving derived classes a chance to provide the property value
            // or alternatively make an async request to obtain it and notify the subscriber via the property events group.ßß

            // For example, this is how we get Media thumbnails for the Media list. If the thumbnail hasn't already been
            // generated, the media actor will do an async request for the thumbnail from the thumbnail service. When it
            // eventually gets the thumbnail it uses 'set_data_value' to update the property and notify subscribers so
            // although the requester here may not get an immediate response, it will eventually receive the property update via the events group.
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

            data_.set_data_value(key, value);
            mail(event_atom_v, property_atom_v, std::vector<int>(), key, value).send(properties_events_group_);
            return true;
        },
        [=](event_atom, property_atom, std::vector<int> index_in_tree, const int key, const AnyPtr &value) {
            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index(current_sender());
            if (cindex < 0) {
                spdlog::debug("Child index not found for sender");
                return;
            }
            // bubble it up
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(event_atom_v, property_atom_v, index_in_tree, key, value).send(properties_events_group_);
        },
        [=](property_atom, const int key) -> result<AnyPtr> {
            return data_.get_data_value(key);
        },
        [=](property_atom) -> result<ObjectDataTreePtr> {

            // Request the full state of the tree rooted at this actor.
            auto auto_responder = AutoResponder<ObjectDataTreePtr>(children_.size(), this);
            auto result = new ObjectDataTree(data_);
            auto_responder.result().reset(result); 
            for (size_t i = 0; i < children_.size(); ++i) {
                mail(property_atom_v).request(children_[i].actor(), infinite).then(
                    [=](ObjectDataTreePtr cdata) mutable {
                        // this const_cast is ok as we 'own' cdata - it was a copy
                        // made by the child and returned to us by this very
                        // message handler (in the child)
                        const_cast<ObjectDataTree *>(cdata.get())->set_parent(result);
                        result->new_node(cdata, i);
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
    