// SPDX-License-Identifier: Apache-2.0
#include <algorithm>

#include "xstudio/timeline/timeline.hpp"
#include "xstudio/utility/helpers.hpp"

using namespace xstudio::timeline;
using namespace xstudio::utility;

Timeline::Timeline(
    const std::string &name, const FrameRate &rate, const Uuid &_uuid, const caf::actor &actor)
    : Item(
          ItemType::IT_TIMELINE,
          UuidActorAddr(_uuid, caf::actor_cast<caf::actor_addr>(actor)),
          {},
          FrameRange(FrameRateDuration(0, rate))) {
    set_name(name);
}

Timeline::Timeline(const JsonStore &jsn)
    : Item(jsn),
      media_list_(static_cast<JsonStore>(jsn.at("media"))) {}

JsonStore Timeline::serialise() const {

    JsonStore jsn = Item::serialise();
    jsn["media"]     = media_list_.serialise();
    return jsn;
}

Timeline Timeline::duplicate() const {
    Timeline tl(serialise());
    tl.set_uuid(utility::Uuid::generate());
    return tl;
}