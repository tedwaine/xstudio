#include "xstudio/broadcast/broadcast_actor.hpp"
#include "xstudio/utility/xs_actor.hpp"

using namespace xstudio::utility;

ObjectDataTreePtr ObjectDataTree::deep_copy() const {
    auto copy = std::make_shared<ObjectDataTree>();
    for (const auto &child : children_data_) {
        copy->children_data_[child.first] = child.second->deep_copy();
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

void ObjectActor::add_child(caf::actor child) {
    int cidx = children_.empty() ? 0 : children_.rbegin()->first + 1;
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
    );

}

void ObjectActor::remove_child(caf::actor child) {
    //children_.erase(std::remove(children_.begin(), children_.end(), caf::actor_cast<caf::actor_addr>(child)), children_.end());
}

caf::message_handler ObjectActor::private_message_handler() {

    auto child_index = [=]() {
        int child_index = -1;
        auto sender = caf::actor_cast<caf::actor_addr>(current_sender());
        auto it = std::find(children_.begin(), children_.end(), sender);
        if (it != children_.end()) {
            child_index = std::distance(children_.begin(), it);
        } else {
            spdlog::warn("{} Received update from unknown child actor: {}", __PRETTY_FUNCTION__, to_string(sender));
        }
        return child_index;
    };

    return caf::message_handler{
        [=](property_atom, const std::string &key, const AnyPtr &value) {
            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index();
            if (cindex < 0) return;
            // bubble it up
            mail(property_atom_v, std::vector<int>{cindex}, value).send(properties_events_group_);
        },
        [=](property_atom, std::vector<int> index_in_tree, const std::string &key, const AnyPtr &value) {
            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index();
            if (cindex < 0) return;
            // bubble it up
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(property_atom_v, index_in_tree, value).send(properties_events_group_);
        },
        [=](property_atom, const ObjectDataTreePtr &child_data) {

            // A child's data has been updated.
            // what's the index of the child that sent this?
            int cindex = child_index();
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
            int cindex = child_index();
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(property_atom_v, index_in_tree, child_data).send(properties_events_group_);

        },
        [=](property_atom) -> ObjectDataTreePtr {

            ObjectDataTree * result = new ObjectDataTree();
            auto auto_responder = AutoResponder<ObjectDataTreePtr>(children_.size());
            auto_responder.result().reset(new ObjectDataTree()); 
            for (size_t i = 0; i < children_.size(); ++i) {
                mail(property_atom_v).request(caf::actor_cast<caf::actor>(children_[i]), infinite).then(
                    [=](ObjectDataTreePtr cdata) {
                        result->children_data_[i] = cdata;
                        auto_responder.decrement();
                    },
                    [=](const caf::error &err) {
                        auto_responder.decrement(err);
                    }
                );
            }
            return auto_responder.response_promise();
        }
    };
}
    