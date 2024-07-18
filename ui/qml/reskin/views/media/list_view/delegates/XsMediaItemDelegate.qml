// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

import "../data_indicators"
import "../../common_delegates"

Rectangle {

    id: contentDiv
    width: parent.width
    implicitHeight: itemRowHeight + drag_target_indicator.height

    color: "transparent"
    property color highlightColor: palette.highlight
    property color bgColorPressed: XsStyleSheet.widgetBgNormalColor
    property color bgColorNormal: "transparent"
    property color forcedBgColorNormal: bgColorNormal
    property color borderColorHovered: highlightColor
    property bool hovered: false
    property color hintColor: XsStyleSheet.hintColor
    property color errorColor: XsStyleSheet.errorColor

    property bool isSelected: false
    property bool isDragTarget: false

    property bool isOnScreen: actorUuidRole == currentPlayhead.mediaUuid

    property var actorUuidRole__: actorUuidRole
    onActorUuidRole__Changed: setSelectionIndex()

    // these are referenced by XsMediaThumbnailImage and XsMediaListMouseArea
    property real mouseX
    property real mouseY

    Connections {
        target: mediaSelectionModel
        function onSelectedIndexesChanged() {
            setSelectionIndex()
        }
    }

    function modelIndex() {
        return helpers.makePersistent(mediaListModelData.rowToSourceIndex(index))
    }

    function setSelectionIndex() {
        var my_idx = modelIndex()
        isSelected = mediaSelectionModel.selectedIndexes.includes(my_idx)
        if (mediaSelectionModel.multiSelected) {
            if (isSelected) {
                selectionIndex = mediaSelectionModel.selectedIndexes.indexOf(my_idx)+1
            } else {
                selectionIndex = 0
            }
        } else {
            selectionIndex = my_idx.row+1
        }
    }

    property int selectionIndex: 0

    property bool playOnClick: false

    Rectangle {
        id: drag_target_indicator
        width: parent.width
        height: visible ? 4 : 0
        visible: isDragTarget
        color: palette.highlight
        Behavior on height { NumberAnimation{duration: 250} }
    }

    Rectangle {
        anchors.fill: parent
        visible: dragTargetIndex != undefined && isSelected
        opacity: 0.5
        color: "white"
        z: 100
    }

    property var mediaSourceMetadataFields: mediaDisplayInfoRole != undefined ? mediaDisplayInfoRole[columns_model_index.row] : []

    property bool fieldsReady: typeof mediaSourceMetadataFields == "object"

    property bool isMissing: false
    property bool isActive: isOnScreen
    property real panelPadding: XsStyleSheet.panelPadding
    property real itemPadding: XsStyleSheet.panelPadding/2

    property real headerThumbWidth: 1

    signal activated() //#TODO: for testing only

    opacity: enabled ? 1.0 : 0.33

    // Note: DelegateChooser has a flaw .. if the 'role' value that drives
    // the choice changes AFTER completion, it does not trigger a switch of
    // the DelegateChoice so I've rolled my own
    Component {

        id: chooser
        Item {
            property var what: data_type
            width: loader.width
            height: loader.height
            onWhatChanged: {
                if (what == "flag") {
                    loader.sourceComponent = flag_indicator
                } else if (what == "thumbnail") {
                    loader.sourceComponent = thumbnail
                } else if (what == "index") {
                    loader.sourceComponent = selection_index
                } else if (what == "notes") {
                    loader.sourceComponent = notes_indicator
                } else {
                    loader.sourceComponent = metadata_value
                }
            }
            Loader {
                id: loader
            }
            Component {
                id: flag_indicator
                XsMediaFlagIndicator{
                    width: size
                    height: itemRowHeight
                }
            }
            Component {
                id: metadata_value
                XsMediaTextItem {
                    text: fieldsReady ? index < mediaSourceMetadataFields.length ? mediaSourceMetadataFields[index] : "" : ""
                    width: size
                    height: itemRowHeight
                }
            }
            Component {
                id: selection_index
                XsMediaTextItem {
                    text: selectionIndex ? selectionIndex : ""
                    width: size
                    height: itemRowHeight
                }
            }
            Component {
                id: notes_indicator
                XsMediaNotesIndicator{
                    width: size
                    height: itemRowHeight
                }
            }
            Component {
                id: thumbnail
                XsMediaThumbnailImage {
                    width: size
                    height: itemRowHeight
                    showBorder: isOnScreen
                }
            }
        }
    }

    DelegateModel {

        id: media_columns_model
        model: columns_model_index.model
        rootIndex: columns_model_index
        delegate: chooser

    }

    Item {

        width: parent.width
        height: itemRowHeight
        y: drag_target_indicator.height

        Rectangle{
            id: rowDividerLine
            width: parent.width;
            height: headerThumbWidth
            color: bgColorPressed
            anchors.bottom: parent.bottom
            z: 100 // on-top of thumbnails etc.
        }

        ListView {
            anchors.fill: parent
            model: media_columns_model
            orientation: ListView.Horizontal
            interactive: false
        }
    }

    //background:
    Rectangle {
        id: bgDiv
        z: -1
        anchors.fill: parent
        border.color: contentDiv.down || contentDiv.hovered ? borderColorHovered: borderColorNormal
        border.width: borderWidth
        color: contentDiv.down || isSelected ? bgColorPressed : forcedBgColorNormal
    }


    function toggleSelection() {

        var myIdx = modelIndex()
        if (!(mediaSelectionModel.selection.count == 1 &&
            mediaSelectionModel.selection[0] == myIdx)) {
            mediaSelectionModel.select(
                myIdx,
                ItemSelectionModel.Toggle
                )
            }

    }

    function exclusiveSelect() {

        mediaSelectionModel.select(
            modelIndex(),
            ItemSelectionModel.ClearAndSelect
            | ItemSelectionModel.setCurrentIndex
            )

    }

    function inclusiveSelect() {

        // For Shift key and select find the nearest selected row,
        // select items between that row and the row of THIS item
        var row = index // row of this item
        var d = 10000
        var nearest_row = -1

        // looping over current selection
        var selection = mediaSelectionModel.selectedIndexes
        for (var i = 0; i < selection.length; ++i) {

            // convert the index of selected item (which is an index into
            // theSessionData) into the row in our filtered media list model
            var r = mediaListModelData.sourceIndexToRow(selection[i])
            if (r > -1) {
                var delta = Math.abs(r-row)
                if (delta < d) {
                    d = delta
                    nearest_row = r
                }
            }
        }

        if (nearest_row!=-1) {

            var first = Math.min(row, nearest_row)
            var last = Math.max(row, nearest_row)
            var selection = []

            for (var i = first; i <= last; ++i) {
                selection.push(helpers.makePersistent(mediaListModelData.rowToSourceIndex(i)))
            }

            mediaSelectionModel.select(
                helpers.createItemSelection(selection),
                ItemSelectionModel.Select
            )
        }
    }

}