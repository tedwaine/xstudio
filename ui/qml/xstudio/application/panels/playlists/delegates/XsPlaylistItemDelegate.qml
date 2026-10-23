// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import xStudio 1.0
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

        property string sourcey: `XsPlaylistItemBase {
        anchors.fill: parent
            }`


    /* Here we have a model to iterate over the contents of the playlist (if
        any) such as subsets, timelines, dividers etc */
    DelegateModel {
        id: subItemsModel

        // we use the main session data model
        // this is required as "model" doesn't issue notifications on change
        //property var notifyModel: theSessionData

        // we use the main session data model
        model: playlistsModelData

        // playlists are one level in at row=0, column=0.
        rootIndex: subItemsModelIndex
        delegate: Item {
            id: pitem
            Layout.fillWidth: true
            Layout.preferredHeight: widget ? widget.implicitHeight : 0
            property var widget
            property var code: qmlCodeRole
            property var modelIndex: playlistsModelData.index(index, 0, subItemsModelIndex)
            onCodeChanged: {
                if (typeof code === "string" && subItemsModelIndex) {
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