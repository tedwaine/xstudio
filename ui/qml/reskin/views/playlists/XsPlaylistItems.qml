// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15
import QuickFuture 1.0

import xStudioReskin 1.0
import "./delegates"

XsListView { id: playlists

    model: playlistsModel

    property var itemsDataModel: null

    property real itemRowStdHeight: XsStyleSheet.widgetStdHeight + 4
    property real subitemIndent: 48
    property real rightSpacing: 6
    property real flagIndicatorWidth: 4

    Rectangle{ id: resultsBg
        anchors.fill: parent
        color: XsStyleSheet.panelBgColor
        z: -1
    }
    
    DelegateModel {
        id: playlistsModel

        // this is required as "model" doesn't issue notifications on change
        property var notifyModel: theSessionData

        // we use the main session data model
        model: notifyModel

        // point at session 0,0, it's children are the playlists.
        rootIndex: notifyModel.playlistsRootIdx
        delegate: chooser
    }

    DelegateChooser {
        id: chooser
        role: "typeRole"

        DelegateChoice {
            roleValue: "ContainerDivider";

            XsPlaylistDividerDelegate{
                modelIndex: helpers.makePersistent(playlistsModel.modelIndex(index))
                width: playlists.width
            }
        }
        DelegateChoice {
            roleValue: "Playlist";

            XsPlaylistItemDelegate {
                modelIndex: helpers.makePersistent(playlistsModel.modelIndex(index))
                width: playlists.width
            }
        }

    }

    XsDragDropHandler {

        id: drag_drop_handler
        targetWidget: playlists
    
        onDropped: {
            
            if (source == "External URIS") {

                var idx = theSessionData.createPlaylist("New Playlist")

                Future.promise(
                    theSessionData.handleDropFuture(
                        Qt.CopyAction,
                        {"text/uri-list": data},
                        idx)
                ).then(function(quuids){
                    mediaSelectionModel.selectNewMedia(index, quuids)
                })
            }
        }
    
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: drag_drop_handler.isDragTarget ? Qt.darker(palette.highlight, 3) : "transparent"
        border.width: 1
    }

}
