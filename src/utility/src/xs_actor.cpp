#include "xstudio/broadcast/broadcast_actor.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/xs_actor.hpp"

using namespace xstudio::utility;

std::set<std::string> &ObjectDataTree::property_names_mutable() {
    // Function-local static: constructed on first use, so safe to call from other TUs' static init.
    static std::set<std::string> names;
    return names;
}

void ObjectDataTree::register_property_names(const std::vector<std::string> &roleNames) {
    auto &names = property_names_mutable();
    for (const auto &roleName : roleNames)
        names.insert(roleName);
}

const std::set<std::string> &ObjectDataTree::property_names() {
    return property_names_mutable();
}

ObjectDataTreePtr ObjectDataTree::deep_copy(ObjectDataTree *parent) const {
    auto copy = std::make_shared<ObjectDataTree>(parent);
    for (const auto &child : children_data_) {
        copy->children_data_[child.first] = child.second->deep_copy(copy.get());
    }
    for (const auto &prop : properties_) {
        copy->properties_[prop.first] = std::make_shared<const std::any>(*prop.second);
    }
    return copy;
}

ObjectActor::ObjectActor(caf::actor_config &cfg) : caf::event_based_actor(cfg) {
    properties_events_group_ = spawn<broadcast::BroadcastActor>(this);
    link_to(properties_events_group_);
}

void ObjectActor::add_child(const UuidActor &child, const utility::Uuid & before) {

    // monitor the child
    monitor(child.actor(), [this, a = child](const error &err) {
        remove_child(a);
        });

    // add the child to the list of children
    auto it = std::find_if(children_.begin(), children_.end(), [before](const UuidActor &c) {
        return c.uuid() == before;
    });
    children_.insert(it, child);

    std::cerr << "Adding child " << to_string(child.actor()) << "\n";
    mail(name_atom_v).request(child.actor(), infinite).then(
        [=](const std::string oink) {
            std::cerr << "oink " << oink << "\n";
            // handle response if needed
        },
        [=](const caf::error &err) {
            spdlog::error("{} Error while joining broadcast: {}", __PRETTY_FUNCTION__, to_string(err));
        }
    );


    // join the child'd property events broadcast group
    mail(property_atom_v, broadcast::join_broadcast_atom_v, caf::actor_cast<caf::actor>(this)).request(child.actor(), infinite).then(
        [=](const bool oink) {
            std::cerr << "SUBSCRIUBED\n";
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
            std::cerr << "GOt child data for index " << cidxs[0] << "\n";
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
    //children_.erase(std::remove(children_.begin(), children_.end(), caf::actor_cast<caf::actor_addr>(child)), children_.end());
}

caf::message_handler ObjectActor::private_message_handler() {

    return caf::message_handler{
        [=](add_children_atom, const utility::UuidActor &child, const utility::Uuid &before) -> bool {
            add_child(child, before);
            return true;
        },
        [=](add_children_atom, const utility::UuidActor &child) -> bool {
            std::cerr << "Adding child " << to_string(child.uuid()) << "\n";
            add_child(child);
            return true;
        },
        [=](property_atom, broadcast::join_broadcast_atom, caf::actor subscriber) -> result <bool> {
            std::cerr << "Joining broadcast with subscriber " << to_string(subscriber) << "\n";
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
        [=](property_atom, const std::string &key, const AnyPtr &value) {
            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            // bubble it up
            mail(property_atom_v, std::vector<int>{cindex}, value).send(properties_events_group_);
        },
        [=](property_atom, std::vector<int> index_in_tree, const std::string &key, const AnyPtr &value) {
            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index(current_sender());
            std::cerr << "BISCUIT " << key << " " << cindex << "\n";
            if (cindex < 0) return;
            // bubble it up
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(property_atom_v, index_in_tree, key, value).send(properties_events_group_);
        },
        [=](property_atom, const ObjectDataTreePtr &child_data) {

            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index(current_sender());
            if (cindex < 0) return;

            /*if (child_index >= 0 && child_index < full_state_.children_data_.size()) {
                full_state_.children_data_[child_index] = child_data;
            }*/
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
        [=](property_atom, const std::string &key, const AnyPtr &value) {
            // A property has been updated.
            int cindex = child_index(current_sender());
            if (cindex < 0) return;
            std::cerr << "INKLO " << key << "\n";
            mail(property_atom_v, std::vector<int>{cindex}, key, value).send(properties_events_group_);
        },
        [=](property_atom) -> result<ObjectDataTreePtr> {

            ObjectDataTree * result = new ObjectDataTree();
            auto auto_responder = utility::AutoResponder<ObjectDataTreePtr>(children_.size(), this);
            auto_responder.result().reset(new ObjectDataTree()); 
            for (size_t i = 0; i < children_.size(); ++i) {
                mail(property_atom_v).request(children_[i].actor(), infinite).then(
                    [=](ObjectDataTreePtr cdata) mutable {
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
    