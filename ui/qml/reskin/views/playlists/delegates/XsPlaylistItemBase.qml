// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.15
import QuickFuture 1.0

import xstudio.qml.helpers 1.0
import xStudioReskin 1.0

Item {
    id: contentDiv

    implicitHeight: itemRowStdHeight

    property real itemPadding: XsStyleSheet.panelPadding/2
    property real buttonWidth: XsStyleSheet.secondaryButtonStdWidth

    property color bgColorPressed: XsStyleSheet.widgetBgNormalColor
    property color bgColorNormal: "transparent"
    property color forcedBgColorNormal: bgColorNormal

    property color highlightColor: palette.highlight
    property color hintColor: XsStyleSheet.hintColor
    property color errorColor: XsStyleSheet.errorColor

    property var iconSource: "qrc:/icons/list_alt.svg"
    property bool indent: false
    property bool isDragTarget: drag_drop_handler.isDragTarget && canReceiveDrag
    property bool canReceiveDrag: true

    // background
    Rectangle {

        id: bgDiv
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: itemRowStdHeight

        border.color: (down || hovered || isDragTarget) ? borderColorHovered : borderColorNormal
        border.width: borderWidth
        color: down || isCurrent ? Qt.darker(palette.highlight, 2) : isSelected ? Qt.darker(palette.highlight, 3) : forcedBgColorNormal

        Rectangle{
            z: -1
            width: parent.width
            height: borderWidth
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            color: XsStyleSheet.menuBarColor
            visible: lineVisible
        }

    }

    // flag
    Rectangle{
        color: flagColourRole
        height: itemRowStdHeight
        width: flagIndicatorWidth
    }

    /* modelIndex should be set to point into the session data model and get
    to the playlist that we are representing */
    property var modelIndex

    onModelIndexChanged: {
        if (modelIndex.valid && modelIndex.row) {
            let prev_item_idx = theSessionData.index(modelIndex.row-1,0,modelIndex.parent)
            lineVisible = theSessionData.get(prev_item_idx, "typeRole") != "ContainerDivider"
        } else {
            lineVisible = true
        }
    }

    /* first index in playlist is media ... */
    property var itemCount: mediaCountRole? mediaCountRole : 0

    property bool isCurrent: modelIndex == inspectedMediaSetIndex
    property bool isSelected: sessionSelectionModel.isSelected(modelIndex)
    property bool isMissing: false
    property bool isExpanded: false
    property bool isExpandable: false
    property bool isViewed: modelIndex == viewedMediaSetIndex
    property bool lineVisible: true
    //property bool mouseOverInspect: false

    property var hovered: ma.containsMouse
    property var down: ma.pressed

    Connections {
        target: sessionSelectionModel // this bubbles up from XsSessionWindow
        function onSelectedIndexesChanged() {
            isSelected = sessionSelectionModel.isSelected(modelIndex)
        }
    }

    MouseArea {

        id: ma
        anchors.fill: bgDiv
        height: parent.height
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        /*onMouseXChanged: checkInspectHover()
        onMouseYChanged: checkInspectHover()

        function checkInspectHover() {
            var pt = mapToItem(inspect_icon, mouseX, mouseY);
            if (pt.x > 0 && pt.x < inspect_icon.width && pt.y > 0 && pt.y < inspect_icon.height) {
                mouseOverInspect = true
            } else {
                mouseOverInspect = false
            }
        }*/

        onPressed: {

            if (mouse.buttons == Qt.RightButton) {
                if (!isSelected) {
                    // right click does an exclusive select, unless
                    // already selected in which case selection doesn't change
                    sessionSelectionModel.setCurrentIndex(
                        modelIndex,
                        ItemSelectionModel.ClearAndSelect
                        )
                }
                showContextMenu(mouseX, mouseY, ma)
            }

            // Put the content of the playlist into the media browser etc.
            // but don't put it on screen.
            if (mouse.modifiers == Qt.ControlModifier) {

                if (!(sessionSelectionModel.selectedIndexes.length == 1 &&
                    sessionSelectionModel.selectedIndexes[0] == modelIndex)) {
                    sessionSelectionModel.select(modelIndex, ItemSelectionModel.Toggle)
                    if (sessionSelectionModel.isSelected(modelIndex)) {
                        sessionSelectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.NoUpdate)
                    }
                }

            } else if (mouse.buttons != Qt.RightButton || (mouse.buttons == Qt.RightButton && !isSelected)) {
                sessionSelectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.ClearAndSelect)
            }

        }

        onDoubleClicked: {
            // Put the content of the playlist into the media browser etc.
            // and also put it on screen
            sessionSelectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.ClearAndSelect)
            viewedMediaSetIndex = helpers.makePersistent(modelIndex)
        }

    }

    property alias mainLayout: layout

    RowLayout {

        id: layout
        anchors.fill: bgDiv
        anchors.leftMargin: indent ? subitemIndent : 0
        anchors.rightMargin: rightSpacing
        spacing: itemPadding

        Item{

            Layout.fillHeight: true
            Layout.margins: 2
            Layout.preferredWidth: indent ? 0 : height

            XsSecondaryButton{

                id: subsetBtn
                z: 100
                imgSrc: "qrc:/icons/chevron_right.svg"
                visible: isExpandable != 0
                anchors.fill: parent
                rotation: (expandedRole)? 90:0
                imageSrcSize: width
                Behavior on rotation {NumberAnimation{duration: 150 }}

                onClicked:{
                    expandedRole = !expandedRole
                }

            }
        }

        XsImage
        {
            Layout.fillHeight: true
            Layout.margins: 2
            width: height
            source: iconSource
            imgOverlayColor: hintColor
        }

        XsText {
            id: textDiv
            text: nameRole
            color: isMissing? hintColor : textColorNormal
            Layout.fillHeight: true
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            font.pixelSize: XsStyleSheet.playlistPanelFontSize
            leftPadding: itemPadding
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            tooltipText: text
            tooltipVisibility: hovered && truncated
            toolTipWidth: contentDiv.width*2
        }

        /*Item {

            Layout.fillHeight: true
            Layout.preferredWidth: height
            Layout.margins: 2

            XsImage {

                id: inspect_icon
                source: "qrc:/icons/visibility.svg"
                visible: contentDiv.hovered || isInspected
                imgOverlayColor: isInspected ? palette.highlight : hintColor
                anchors.fill: parent
                anchors.margins: 2

            }

            Rectangle {

                anchors.fill: parent
                color: "transparent"
                border.color: palette.highlight
                border.width: borderWidth
                visible: hoeverdOnInspect
            }

        }*/

        XsImage {

            id: inspect_icon
            source: "qrc:/icons/desktop_windows.svg"
            visible: isViewed
            imgOverlayColor: palette.highlight
            Layout.fillHeight: true
            Layout.preferredWidth: height
            Layout.margins: 2

        }

        XsText{
            id: countDiv
            text: itemCount
            Layout.minimumWidth: buttonWidth + 5
            Layout.preferredHeight: buttonWidth
            color: hintColor
        }

        Item{

            Layout.preferredWidth: errorIndicator.visible ? buttonWidth : 0
            Layout.preferredHeight: errorIndicator.visible ? buttonWidth : 0

            XsSecondaryButton{
                id: errorIndicator
                anchors.fill: parent
                visible: errorRole != 0
                imgSrc: "qrc:/icons/error.svg"
                imgOverlayColor: hintColor

                toolTip.text: errorRole +" errors"
                toolTip.visible: hovered
            }
        }
    }

    XsDragDropHandler {

        id: drag_drop_handler
        targetWidget: bgDiv

        dragSourceName: "PlayList"
        dragData: sessionSelectionModel.selectedIndexes

        onDragEntered: {

            if (source == "MediaList") {

                // User is dragging selected media from a MediaList into
                // an item in the Playlists panel

                // check if the user is dragging from a playlist or playlist
                // child into the same child or the parent playlist
                // (note, the parent of the items in data will be 'MediaList'.
                // The parent of the MediaList is the Subset, Timeline, Playlist.
                // If the media is from a subset, say, it's parent's parent's
                // parent's parent will be the Playlist ... we also check for an
                // attempt to drop from subset into parent playlist! - these
                // wil be no-ops)

                if (data.length &&
                    (data[0].parent.parent == modelIndex ||
                        data[0].parent.parent.parent.parent == modelIndex)) {

                    canReceiveDrag = false

                } else {

                    canReceiveDrag = true
                }
            } else if (source == "External") {

                canReceiveDrag = true

            }
        }

        function doMove(button, data) {
            if (button != "Cancel") {
                let type = modelIndex.model.get(modelIndex, "typeRole")
                let new_indexes = theSessionData.moveRows(
                    data,
                    -1, // insertion row: make invalid so always inserts on the end
                    modelIndex,
                    button == "Copy"
                )

                if(type == "Timeline" && new_indexes.length) {
                    // insert new media as new tracks.
                    // find stack..

                    let tindex = theSessionData.index(2, 0, modelIndex)
                    theSessionData.fetchMoreWait(tindex)
                    let sindex = theSessionData.index(0, 0, tindex)

                    let newvindex = theSessionData.insertRowsSync(0, 1, "Video Track", "Dropped", sindex)[0];
                    let newaindex = theSessionData.insertRowsSync(theSessionData.rowCount(sindex), 1 ,"Audio Track", "Dropped", sindex)[0];

                    for(let i = 0; i < new_indexes.length;i++) {
                        theSessionData.insertTimelineClip(i, newvindex, new_indexes[i], "")
                        theSessionData.insertTimelineClip(i, newaindex, new_indexes[i], "")
                    }
                }
            }
        }

        onDropped: {

            if (!isDragTarget) return
            isDragTarget = false
            if (source == "MediaList") {
                if (data[0].parent.parent == modelIndex.parent.parent) {
                    // here the source media is from the same playlist as the
                    // parent playlist (i.e. copy from parent playlist into a
                    // child (subset/timeline) of the same playlist)
                    doMove("Copy", data)
                } else {
                    dialogHelpers.multiChoiceDialog(
                        doMove,
                        "Copy/Move Media",
                        "Do you want to move or copy the media?",
                        ["Cancel", "Copy", "Move"],
                        data)
                }
            } else if (source == "External URIS") {
                Future.promise(
                    theSessionData.handleDropFuture(Qt.CopyAction, {"text/uri-list": data}, modelIndex)
                ).then(function(quuids){
                    mediaSelectionModel.selectNewMedia(index, quuids)
                })
            }
        }

    }


}