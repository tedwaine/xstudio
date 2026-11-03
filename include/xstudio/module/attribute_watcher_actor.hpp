#pragma once

#include "xstudio/utility/xs_actor.hpp"
#include "xstudio/utility/logging.hpp"

namespace xstudio::module {

/**
 * AttributeWatcherActor allows us to expose Module attributes to the UI layer and handle attribute change requests.
 * We base from ObjectActor structure which can be added as a child to another ObjectActor that represents a 
 * 'group' of attributes. In the UI we can use XsSessionModelNode which exposes the parent ObjectActor and its
 * children via a QAbstractItemModel which is the most efficient way to represent hierarchical data in the UI.
 */
class AttributeWatcherActor : public utility::ObjectActor {

    utility::Uuid attribute_uuid_;
    caf::actor module;

    public:

    AttributeWatcherActor(
        caf::actor_config &cfg,
        const utility::PropertyMap &attribute_data,
        const utility::Uuid &attribute_uuid,
        caf::actor module): utility::ObjectActor(cfg, to_string(attribute_uuid)), attribute_uuid_(attribute_uuid), module(module) {
        for (const auto &p: attribute_data) {
            // Process each role data entry here
            set_data_value(p.first, p.second);
        }
    }

    protected:

    caf::message_handler message_handler() override {

        return {
            [=](change_attribute_event_atom, const int role, const utility::AnyPtr &data_value) {
                try {
                    set_data_value(role, data_value);
                } catch (std::exception &e) {
                    spdlog::error("Exception in AttributeWatcherActor: {}", e.what());
                }
            }
        };
    }

    bool request_to_set_data_value(const int key, const utility::AnyPtr &value) override {
        // This method is called when the UI layer is tring to change an attribute
        // value. We check if the change is allowed.

        // Tell the Module (backend) to update the correponding attribute with the new value.
        // If the change is successful, the backend will notify us via the attribute_changed_atom message.
        anon_mail(change_attribute_request_atom_v, attribute_uuid_, key, value).send(module);
        
        // no immediate change is made locally; we wait for the backend to confirm the change.
        return false;
    }

};

} // namespace xstudio::module
