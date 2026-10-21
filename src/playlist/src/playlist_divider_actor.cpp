// SPDX-License-Identifier: Apache-2.0

#include "xstudio/playlist/playlist_divider_actor.hpp"

using namespace xstudio;
using namespace xstudio::playlist;

PlaylistDividerActor::PlaylistDividerActor(
    caf::actor_config &cfg, const utility::JsonStore &jsn,
    const caf::actor &/*parent*/)
    : xstudio::utility::ObjectActor(cfg),
      utility::Container(utility::JsonStore(jsn.at("base"))) {

    

}

PlaylistDividerActor::PlaylistDividerActor(
    caf::actor_config &cfg,
    const std::string &name,
    const utility::Uuid &uuid,
    const caf::actor &/*parent*/)
    : xstudio::utility::ObjectActor(cfg),
      Container(name, "ContainerDivider", uuid) {

    

}

namespace {
static const char * session_list_qml = R"qml(
    import QtQuick
    import xStudio 1.0

    XsPlaylistDividerDelegate {
    }
)qml";
}
REGISTER_SESSION_OBJECT(PlaylistDividerActor, ContainerDivider, session_list_qml)
    