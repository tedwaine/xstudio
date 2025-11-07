// SPDX-License-Identifier: Apache-2.0

#include "xstudio/timeline/gap.hpp"

using namespace xstudio;
using namespace xstudio::timeline;
using namespace xstudio::utility;

Gap::Gap(
    const std::string &name,
    const FrameRateDuration &duration,
    const Uuid &_uuid,
    const caf::actor &actor)
    : Item(
          ItemType::IT_GAP,
          UuidActorAddr(_uuid, caf::actor_cast<caf::actor_addr>(actor)),
          FrameRange(FrameRateDuration(0, duration.rate()), duration),
          FrameRange(FrameRateDuration(0, duration.rate()), duration)) {
    set_name(name);
}

Gap::Gap(const JsonStore &jsn)
    : Item(jsn) {}

Gap::Gap(const Item &item, const caf::actor &actor)
    : Item(item.clone()) {
    Item::set_actor_addr(caf::actor_cast<caf::actor_addr>(actor));
}

Gap Gap::duplicate() const {
    Gap gap(serialise());
    gap.set_uuid(utility::Uuid::generate());
    return gap;
}
