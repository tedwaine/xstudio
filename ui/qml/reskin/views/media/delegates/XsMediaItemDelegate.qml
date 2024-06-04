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

    property bool isSelected: mediaSelectionModel.selectedIndexes.includes(media_item_model_index)
    property bool isDragTarget: media_item_model_index == dragTargetIndex

    property bool isOnScreen: actorUuidRole == currentPlayhead.mediaUuid

    property var selectionIndex: mediaSelectionModel.multiSelected ? mediaSelectionModel.selectedIndexes.indexOf(media_item_model_index)+1 : media_item_model_index.row+1

    // get the index into the session model for the MediaSource (image)
    property var imageSourceUuid: media_item_model_index.valid ? media_item_model_index.model.get(media_item_model_index, "imageActorUuidRole") : ""
    property var imageSourceIndex: media_item_model_index.valid && imageSourceUuid ? media_item_model_index.model.searchRecursive(
                                        imageSourceUuid,
                                        "actorUuidRole",
                                        media_item_model_index) : media_item_model_index

    property real mouseX
    property real mouseY
    property bool playOnClick: false

    onImageSourceIndexChanged: {
        // horrible shenanegans!
        if (imageSourceIndex.valid) {
            theSessionData.fetchMore(imageSourceIndex)
            callbackTimer.setTimeout(function(plindex, obj) { return function() {
                obj.imageStreamIndex = plindex.model.index(0,0,plindex.model.index(0,0,plindex))
            }}( imageSourceIndex, contentDiv ), 200);
        }
    }
    
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
    
    // To index to the image stream we need to go two levels deeper into the model as
    // it looks like this:
    // Media {
    //    MediaSource {
    //        Image Stream {
    //            MediaStream {}
    //        }
    //        Audio Stream {
    //            MediaStream {}
    //        }
    //    }
    //    MediaSource { ... }
    //    MediaSource { ... }
    //}
    property var imageStreamIndex: imageSourceIndex.valid? imageSourceIndex.model.index(0,0,imageSourceIndex.model.index(0,0,imageSourceIndex)) : undefined

    property var mediaSourceMetadataFields: mediaDisplayInfoRole != undefined ? mediaDisplayInfoRole[columns_model_index.row] : []

    property bool isMissing: false
    property bool isActive: isOnScreen
    property real panelPadding: XsStyleSheet.panelPadding
    property real itemPadding: XsStyleSheet.panelPadding/2

    property real headerThumbWidth: 1

    // property real rowHeight:  XsStyleSheet.widgetStdHeight
    property real itemHeight: (rowHeight-8) //16

    signal activated() //#TODO: for testing only

    //font.pixelSize: textSize
    //font.family: textFont
    //hoverEnabled: true
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
                    text: index < mediaSourceMetadataFields.length ? mediaSourceMetadataFields[index] : ""
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

        if (!(mediaSelectionModel.selection.count == 1 &&
            mediaSelectionModel.selection[0] == media_item_model_index)) {
            mediaSelectionModel.select(
                media_item_model_index,
                ItemSelectionModel.Toggle
                )
            }

    }

    function exclusiveSelect() {

        mediaSelectionModel.select(
            media_item_model_index,
            ItemSelectionModel.ClearAndSelect
            | ItemSelectionModel.setCurrentIndex
            )

    }

    function inclusiveSelect() {

        // For Shift key and select find the nearest selected row,
        // select items between that row and the row of THIS item
        var row = media_item_model_index.row
        var d = 10000
        var nearest_row = -1
        var selection = mediaSelectionModel.selectedIndexes

        for (var i = 0; i < selection.length; ++i) {
            var delta = Math.abs(selection[i].row-row)
            if (delta < d) {
                d = delta
                nearest_row = selection[i].row
            }
        }

        if (nearest_row!=-1) {

            var model = media_item_model_index.model
            var first = Math.min(row, nearest_row)
            var last = Math.max(row, nearest_row)
            var selection = []

            for (var i = first; i <= last; ++i) {

                selection.push(model.index(
                    i,
                    media_item_model_index.column,
                    media_item_model_index.parent
                    ))
            }

            mediaSelectionModel.select(
                helpers.createItemSelection(selection),
                ItemSelectionModel.ClearAndSelect
            )
        }
    }


    // onClicked: {
    //     isSelected = true
    // }
    /*onDoubleClicked: {
        isSelected = true
        activated() //#TODO
    }
    onPressed: {
        mediaSelectionModel.select(media_item_model_index, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.setCurrentIndex)
    }
    onReleased: {
        focus = false
    }
    onPressAndHold: {
        isMissing = !isMissing
    }*/

}