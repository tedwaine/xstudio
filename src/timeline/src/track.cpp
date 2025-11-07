// SPDX-License-Identifier: Apache-2.0
#include <algorithm>

#include "xstudio/timeline/track.hpp"
#include "xstudio/utility/helpers.hpp"

using namespace xstudio;
using namespace xstudio::media;
using namespace xstudio::timeline;
using namespace xstudio::utility;

Track::Track(
    const std::string &name,
    const FrameRate &rate,
    const MediaType media_type,
    const Uuid &_uuid,
    const caf::actor &actor)
    : Item(
          media_type == MediaType::MT_IMAGE ? ItemType::IT_VIDEO_TRACK
                                            : ItemType::IT_AUDIO_TRACK,
          UuidActorAddr(_uuid, caf::actor_cast<caf::actor_addr>(actor)),
          {},
          FrameRange(FrameRateDuration(0, rate))), media_type_(media_type) {
    Item::set_name(name);
}

Track::Track(const JsonStore &jsn)
    : Item(jsn) {
    media_type_ = jsn.at("media_type");
}

Track::Track(const Item &item, const caf::actor &actor)
    : Item(item.clone()) {
    Item::set_actor_addr(caf::actor_cast<caf::actor_addr>(actor));
    media_type_ =
        (Item::item_type() == ItemType::IT_VIDEO_TRACK ? MediaType::MT_IMAGE
                                                       : MediaType::MT_AUDIO);
}

Track Track::duplicate() const {

    Track tk(serialise());
    tk.set_uuid(utility::Uuid::generate());
    return tk;
}

JsonStore Track::serialise() const {
    JsonStore jsn = Item::serialise();
    jsn["media_type"] = media_type_;
    return jsn;
}

void Track::set_media_type(const MediaType media_type) { media_type_ = media_type; }
