// SPDX-License-Identifier: Apache-2.0

#include "xstudio/timeline/clip.hpp"

using namespace xstudio::timeline;
using namespace xstudio;

Clip::Clip(
    const std::string name,
    const utility::Uuid &_uuid,
    const utility::Uuid media_uuid)
    : Item(ItemType::IT_CLIP, _uuid),
      media_uuid_(std::move(media_uuid)) {
    Item::set_name(name);

    auto jsn          = R"({"media_uuid": null})"_json;
    jsn["media_uuid"] = media_uuid_;
    Item::set_prop(utility::JsonStore(jsn));
}

Clip::Clip(const utility::JsonStore &jsn)
    : Item(jsn) {

    // hack for old data
    if (jsn.count("media_uuid")) {
        media_uuid_       = jsn.at("media_uuid");
        auto jsn          = R"({"media_uuid": null})"_json;
        jsn["media_uuid"] = media_uuid_;
        Item::set_prop(utility::JsonStore(jsn));
    } else {
        media_uuid_ = Item::prop().at("media_uuid");
    }

    if (jsn.count("overriden_media_rate")) {

        overridden_media_rate_ = jsn.at("overriden_media_rate");
    }
}

Clip::Clip(const Item &item, const caf::actor &actor)
    : Item(item) {
    media_uuid_ = Item::prop().value("media_uuid", utility::Uuid());
    Item::set_actor_addr(caf::actor_cast<caf::actor_addr>(actor));
}


Clip Clip::duplicate() const {

    Clip dup(serialise());
    dup.set_uuid(utility::Uuid::generate());
    return dup;
}

utility::JsonStore Clip::serialise() const {
    utility::JsonStore jsn = Item::serialise();
    jsn["overriden_media_rate"] = overridden_media_rate_;
    return jsn;
}
