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
        caf::actor module);

    protected:

    caf::message_handler message_handler() override;

    bool request_to_set_data_value(const int key, const utility::AnyPtr &value) override;

};

} // namespace xstudio::module
