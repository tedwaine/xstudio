// SPDX-License-Identifier: Apache-2.0

#include "xstudio/timeline/gap.hpp"

using namespace xstudio;
using namespace xstudio::timeline;
using namespace xstudio::utility;

Gap::Gap(
    const std::string &name,
    const FrameRateDuration &duration,
    const Uuid &_uuid)
    : Item(
          ItemType::IT_GAP,
          _uuid,
          FrameRange(FrameRateDuration(0, duration.rate()), duration),
          FrameRange(FrameRateDuration(0, duration.rate()), duration)) {
    set_name(name);
}

Gap::Gap(const JsonStore &jsn)
    : Item(jsn) {}

Gap::Gap(const Item &item)
    : Item(item.clone()) {
}

Gap Gap::duplicate() const {
    Gap gap(serialise());
    gap.set_uuid(utility::Uuid::generate());
    return gap;
}
