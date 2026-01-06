// SPDX-License-Identifier: Apache-2.0

#include "xstudio/timeline/stack.hpp"

using namespace xstudio;
using namespace xstudio::timeline;
using namespace xstudio::utility;

Stack::Stack(
    const std::string &name, const FrameRate &rate, const Uuid &uuid_)
    : Item(
          ItemType::IT_STACK,
          uuid_,
          {},
          FrameRange(FrameRateDuration(0, rate))) {
    set_name(name);
}

Stack::Stack(const JsonStore &jsn)
    : Item(jsn) {}

Stack::Stack(const Item &item)
    : Item(item.clone()) {
}

Stack Stack::duplicate() const {
    Stack stk(serialise());
    stk.set_uuid(utility::Uuid::generate());
    stk.reset_uuid(true);
    return stk;
}