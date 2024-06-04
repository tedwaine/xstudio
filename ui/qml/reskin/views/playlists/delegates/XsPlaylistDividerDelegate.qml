// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import xstudio.qml.helpers 1.0
import xStudioReskin 1.0

Item {
    id: contentDiv
    width: parent.width;
    height: itemRowStdHeight

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
    
    // background
    Rectangle {

        id: bgDiv
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: itemRowStdHeight

        border.color: (down || hovered) ? borderColorHovered : borderColorNormal
        border.width: borderWidth
        color: down || isCurrent ? Qt.darker(palette.highlight, 2) : isSelected ? Qt.darker(palette.highlight, 3) : forcedBgColorNormal
        
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

    /* first index in playlist is media ... */
    property var itemCount: mediaCountRole? mediaCountRole : 0

    property bool isCurrent: modelIndex == inspectedMediaSetIndex
    property bool isSelected: sessionSelectionModel.isSelected(modelIndex)
    property bool isMissing: false
    property bool isExpanded: false
    property bool isExpandable: false
    property bool isViewed: modelIndex == viewedMediaSetIndex
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

        onPressed: {

            if (mouse.buttons == Qt.RightButton) {
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

    }

    RowLayout {

        id: layout
        anchors.fill: bgDiv
        anchors.leftMargin: indent ? subitemIndent+itemPadding*6 : rightSpacing
        anchors.rightMargin: rightSpacing
        spacing: 10

        Rectangle{ 
            height: 1; 
            color: hintColor
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }
        
        XsText {
            
            id: textDiv
            text: nameRole
            color: hintColor 
            Layout.alignment: Qt.AlignVCenter

        }

        Rectangle{ 
            height: 1; 
            color: hintColor
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }


    }

}