#include "xstudio/broadcast/broadcast_actor.hpp"
#include "xstudio/utility/xs_actor.hpp"

using namespace xstudio::utility;

ObjectDataTreePtr ObjectDataTree::deep_copy() const {
    auto copy = std::make_shared<ObjectDataTree>();
    for (const auto &child : children_data_) {
        copy->children_data_.push_back(child->deep_copy());
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
    children_.push_back(caf::actor_cast<caf::actor_addr>(child));
    mail(property_atom_v).request(child, infinite).then(
        [=](const ObjectDataTreePtr &child_data) mutable {
            full_state_.children_data_.push_back(child_data);
            std::vector<int> child_index{int(full_state_.children_data_.size()-1)};
            mail(property_atom_v, child_index, child_data).send(properties_events_group_);
        },
        [=](const caf::error &err) {
            spdlog::error("{} Error while requesting child data: {}", __PRETTY_FUNCTION__, to_string(err));
        }
    );

}

void ObjectActor::remove_child(caf::actor child) {
    children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
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

            // Data has been updated of a child's child
            // what's the index of the child that sent this?
            int cindex = child_index();
            if (cindex < 0) return;
            index_in_tree.insert(index_in_tree.begin(), cindex);
            mail(property_atom_v, index_in_tree, child_data).send(properties_events_group_);

        },
        [=](property_atom) -> ObjectDataTreePtr {
            return full_state_.deep_copy();
        }
    };
}
    