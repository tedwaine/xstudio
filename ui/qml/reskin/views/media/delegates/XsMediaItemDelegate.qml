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
    width: parent.width;
    height: parent.height

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

    property bool isOnScreen: mediaUuid == onScreenMediaUuid

    property var selectionIndex: mediaSelectionModel.multiSelected ? mediaSelectionModel.selectedIndexes.indexOf(media_item_model_index)+1 : media_item_model_index.row+1

    // get the index into the session model for the MediaSource (image)
    property var imageSourceUuid: media_item_model_index.model.get(media_item_model_index, "imageActorUuidRole")
    property var imageSourceIndex: media_item_model_index.model.searchRecursive(
                                        imageSourceUuid,
                                        "actorUuidRole",
                                        media_item_model_index)

    onImageSourceIndexChanged: {
        // horrible shenanegans!
        if (imageSourceIndex.valid) {
            theSessionData.fetchMore(imageSourceIndex)
            callbackTimer.setTimeout(function(plindex, obj) { return function() {
                obj.imageStreamIndex = plindex.model.index(0,0,plindex.model.index(0,0,plindex))
            }}( imageSourceIndex, contentDiv ), 200);
        }
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

    property var mediaSourceMetadataFields: metadataSet0Role

    XsModelProperty {
        id: imageStreamMeta
        role: "metadataSet0Role"
        index: imageStreamIndex
    }

    property var mediaStreamMetadataFields: imageStreamMeta.value

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

    property var columns_model

    Item {

        anchors.fill: parent

        Rectangle{
            id: rowDividerLine
            width: parent.width;
            height: headerThumbWidth
            color: bgColorPressed
            anchors.bottom: parent.bottom
            z: 100 // on-top of thumbnails etc.
        }

        RowLayout{

            id: row
            spacing: 0
            height: parent.height

            Repeater {

                // Note: columns_model is set-up in the ui_qml.json preference
                // file. Look for 'media_list_columns_config' item in that
                // file. It specifies the title, size, data_type and so-on for
                // each column in the media list view. The DelegateChooser
                // here creates graphics/text items that go into the media list
                // table depedning on the 'data_type'. To add new ways to view
                // data like traffic lights, icons and so-on create a new
                // indicator class with a new correspondinf 'data_type' in the
                // ui_qml.json
                model: columns_model
                delegate: chooser

                DelegateChooser {

                    id: chooser
                    role: "data_type"

                    DelegateChoice {
                        roleValue: "flag"
                        
                        XsMediaFlagIndicator{
                            Layout.preferredWidth: size
                            Layout.minimumHeight: itemHeight
                        }
                    }

                    DelegateChoice {
                        roleValue: "metadata"
                        // we might want to pull metadata from the Media item (e.g. pipeline status of an asset/render)
                        // Or we might want to pull metadata from the MediaSource (e.g. filesystem date stamp)
                        // Or me might want to pull metadata from the MediaStream (e.g. codec name)
                        XsMediaTextItem {
                            property var mediaMetadataField: mediaItemMetadataFields ? mediaItemMetadataFields[index] : ""
                            property var mediaSourceMetadataField: mediaSourceMetadataFields ? mediaSourceMetadataFields[index] : ""
                            property var imageStreameMetadataField: mediaStreamMetadataFields ? mediaStreamMetadataFields[index] : "--"
                            raw_text: mediaMetadataField ? mediaMetadataField : mediaSourceMetadataField ? mediaSourceMetadataField : imageStreameMetadataField
                            Layout.preferredWidth: size
                            Layout.minimumHeight: itemHeight
                        }
                    }

                    DelegateChoice {
                        roleValue: "role_data"

                        XsMediaTextItem {
                            Layout.preferredWidth: size
                            Layout.minimumHeight: itemHeight
                            raw_text: "" + modelProperty.value;
                            XsModelProperty {
                                id: modelProperty
                                role: role_name
                                index: object == "MediaStream" ? imageStreamIndex : object == "MediaSource" ? imageSourceIndex : media_item_model_index
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "index"

                        XsMediaTextItem {
                            text: selectionIndex ? selectionIndex : ""
                            Layout.preferredWidth: size
                            Layout.minimumHeight: itemHeight
                        }
                    }

                    DelegateChoice {
                        roleValue: "notes"
                        
                        XsMediaNotesIndicator{
                            Layout.preferredWidth: size
                            Layout.minimumHeight: itemHeight
                        }
                    }

                    DelegateChoice {
                        roleValue: "thumbnail"

                        XsMediaThumbnailImage {
                            Layout.preferredWidth: size
                            Layout.fillHeight: true
                            showBorder: isOnScreen
                        }
                    }

                }

            }
        }
    }

    //background:
    Rectangle {
        id: bgDiv
        anchors.fill: parent
        border.color: contentDiv.down || contentDiv.hovered ? borderColorHovered: borderColorNormal
        border.width: borderWidth
        color: contentDiv.down || isSelected ? bgColorPressed : forcedBgColorNormal

        Rectangle {
            id: bgFocusDiv
            implicitWidth: parent.width+borderWidth
            implicitHeight: parent.height+borderWidth
            visible: contentDiv.activeFocus
            color: "transparent"
            opacity: 0.33
            border.color: borderColorHovered
            border.width: borderWidth
            anchors.centerIn: parent
        }

        // Rectangle{anchors.fill: parent; color: "grey"; opacity:(index%2==0?.2:0)}
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