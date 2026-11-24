// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import xStudio 1.0
import xstudio.qml.session 1.0
import "."

XsPlaylistItemBase {

    isExpandable: subItemsCount != 0
    implicitHeight: itemRowStdHeight + subItems.height + indicator_height
    clip: true
    id: root

    Behavior on implicitHeight {NumberAnimation{duration: 100 }}

    /* .... the third row gives us the data of the subsets/timelines etc. i.e.
    the children lists of the playlist */
    property var subItemsModelIndex: modelIndex// && modelIndex.valid ? theSessionData.index(2, 0, modelIndex) : undefined
    property var subItemsCount: subItemsModel.count

    /* Here we have a model to iterate over the contents of the playlist (if
        any) such as subsets, timelines, dividers etc */
    DelegateModel {
        id: subItemsModel

        // we use the main session data model
        // this is required as "model" doesn't issue notifications on change
        //property var notifyModel: theSessionData

        // we use the main session data model
        model: playlistsModelData

        // Containers (Subset, ContactSheet, Divider etc.) are grouped under row=1, column=0 in the playlists model.
        rootIndex: playlistsModelData.index(1, 0, subItemsModelIndex)

        delegate: Item {
            id: pitem
            Layout.fillWidth: true
            Layout.preferredHeight: widget ? widget.implicitHeight : 0
            property var widget
            property var code: qmlCodeRole
            property var modelIndex: playlistsModelData.index(index, 0, subItemsModel.rootIndex)
            onCodeChanged: {
                if (typeof code === "string") {
                    widget = Qt.createQmlObject(code, pitem)
                    widget.indent = true
                }
            }
        }

    }

    // The layout to show the playlist sub-items
    ColumnLayout {
        id: subItems
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: itemRowStdHeight + indicator_height
        spacing: 0

        Repeater {

            model: expandedRole ? subItemsModel : []

        }
    }

}