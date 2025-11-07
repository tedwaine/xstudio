// SPDX-License-Identifier: Apache-2.0

#include "xstudio/timeline/stack.hpp"

using namespace xstudio;
using namespace xstudio::timeline;
using namespace xstudio::utility;

Stack::Stack(
    const std::string &name, const FrameRate &rate, const Uuid &uuid_, const caf::actor &actor)
    : Item(
          ItemType::IT_STACK,
          UuidActorAddr(uuid_, caf::actor_cast<caf::actor_addr>(actor)),
          {},
          FrameRange(FrameRateDuration(0, rate))) {
    set_name(name);
}

Stack::Stack(const JsonStore &jsn)
    : Item(jsn) {}

Stack::Stack(const Item &item, const caf::actor &actor)
    : Item(item.clone()) {
    set_actor_addr(caf::actor_cast<caf::actor_addr>(actor));
}

Stack Stack::duplicate() const {
    Stack stk(serialise());
    stk.set_uuid(utility::Uuid::generate());
    return stk;
}