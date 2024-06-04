// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQml.Models 2.15
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import "."

XsPlaylistItemBase {

    id: contentDiv
    isExpandable: subItemsCount != 0

    implicitHeight: itemRowStdHeight + (expandedRole ? subItemsCount*itemRowStdHeight : 0)

    Behavior on implicitHeight {NumberAnimation{duration: 50 }}

    // when a playlist is freshly created the corresponding node in the model
    // is not fully populated with the playlist structure (and the placeholderRole
    // is true). The node data is filled out when the backend has notified the
    // model with the playlist data and placeholderRole becomes false.
    property var is_placeholder: placeHolderRole
    onIs_placeholderChanged: {
        subItemsModelIndex = theSessionData.index(2, 0, modelIndex)
    }

    /* .... the third row gives us the data of the subsets/timelines etc. i.e.
    the children lists of the playlist */
    property var subItemsModelIndex: modelIndex && modelIndex.valid ? theSessionData.index(2, 0, modelIndex) : undefined
    property var subItemsCount: subItemsModel.count

    /* Here we have a model to iterate over the contents of the playlist (if
        any) such as subsets, timelines, dividers etc */
    DelegateModel {
        id: subItemsModel

        // we use the main session data model
        // this is required as "model" doesn't issue notifications on change
        property var notifyModel: theSessionData

        // we use the main session data model
        model: notifyModel

        // playlists are one level in at row=0, column=0.
        rootIndex: subItemsModelIndex
        delegate: chooser

    }

    DelegateChooser {
        id: chooser
        role: "typeRole"

        DelegateChoice {

            roleValue: "Subset"
            XsSubsetItemDelegate{
                modelIndex: subItemsModel.modelIndex(index)
                Layout.fillWidth: true
            }
        }

        DelegateChoice {

            roleValue: "Timeline"
            XsTimelineItemDelegate{
                modelIndex: subItemsModel.modelIndex(index)
                Layout.fillWidth: true
            }
        }

        DelegateChoice {

            roleValue: "ContainerDivider"
            XsPlaylistDividerDelegate {
                modelIndex: subItemsModel.modelIndex(index)
                indent: true
                Layout.fillWidth: true
            }

        }

    }

    // The layout to show the playlist sub-items
    ColumnLayout {

        id: subItems
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: mainLayout.bottom
        visible: expandedRole ? expandedRole : false
        spacing: 0

        Repeater {

            model: subItemsModel

        }
    }

}