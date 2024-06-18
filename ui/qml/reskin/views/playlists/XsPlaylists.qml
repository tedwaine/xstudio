// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import "./widgets"

Item{

    id: panel

    anchors.fill: parent

    property color panelColor: XsStyleSheet.panelBgColor
    property color bgColorPressed: palette.highlight
    property color bgColorNormal: "transparent"
    property color forcedBgColorNormal: bgColorNormal
    property color borderColorHovered: bgColorPressed
    property color borderColorNormal: "transparent"
    property real borderWidth: 1
    property bool isSubDivider: false

    property real textSize: XsStyleSheet.fontSize
    property var textFont: XsStyleSheet.fontFamily
    property color textColorNormal: palette.text
    property color hintColor: XsStyleSheet.hintColor

    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real secBtnWidth: XsStyleSheet.secondaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding

    XsGradientRectangle{
        anchors.fill: parent
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        propagateComposedEvents: true
        acceptedButtons: Qt.RightButton
        onPressed: {
            if (mouse.buttons == Qt.RightButton) {
                showContextMenu(mouseX, mouseY, ma)
            }
        }
    }

    ColumnLayout {

        id: titleDiv
        anchors.fill: parent
        anchors.margins: panelPadding

        RowLayout{

            x: panelPadding
            spacing: 1
            Layout.fillWidth: true

            XsPrimaryButton{ id: addPlaylistBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/add.svg"
                onClicked: {
                    var pos = mapToItem(panel, x+width/2, y+height/2)
                    showMenu(pos.x ,pos.y)
                }
            }

            XsPrimaryButton{ id: deleteBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/delete.svg"
                onClicked: {
                    removeSelected()
                }
            }
            // XsSearchButton{ id: searchBtn
            //     Layout.preferredWidth: isExpanded? btnWidth*6 : btnWidth
            //     Layout.preferredHeight: btnHeight
            //     isExpanded: false
            //     hint: "Search playlists..."
            // }
            XsText {
                Layout.fillWidth: true
                Layout.preferredHeight: btnHeight
                elide: Text.ElideMiddle
                text: filename
                font.bold: true
                property string path: sessionProperties.values.pathRole ? sessionProperties.values.pathRole : ""
                property string filename: path ? path.substring(path.lastIndexOf("/")+1) : sessionProperties.values.nameRole ? sessionProperties.values.nameRole : ""
            }

            XsPrimaryButton{ 
                id: morePlaylistBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/more_vert.svg"
                onClicked: {
                    showContextMenu(width/2, height/2, morePlaylistBtn)
                }
            }
        }

        XsPlaylistItems{

            id: playlistItems
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }

    Loader {
        id: menu_loader
    }

    Component {
        id: plusMenuComponent
        XsPlaylistPlusMenu {
        }
    }

    function showMenu(mx, my) {
        if (menu_loader.item == undefined) {
            menu_loader.sourceComponent = plusMenuComponent
        }
        menu_loader.item.x = mx
        menu_loader.item.y = my
        menu_loader.item.visible = true
    }

    Loader {
        id: context_menu_loader
    }

    Component {
        id: contextMenuComponent
        XsPlaylistContextMenu {
        }
    }

    function removeSelected() {

        dialogHelpers.multiChoiceDialog(
            doRemove,
            "Remove Selected Items",
            "Remove selected items?",
            ["Cancel", "Remove Items"], 
            undefined)

    }

    function doRemove(button) {

        if (button == "Cancel") return

        let perst_indeces = []
        for (var i = 0; i < sessionSelectionModel.selectedIndexes.length; ++i) {
            let index = sessionSelectionModel.selectedIndexes[i]
            perst_indeces.push(helpers.makePersistent(index))
        }

        sessionSelectionModel.clear()

        for (var i = 0; i < perst_indeces.length; ++i) {
            theSessionData.removeRows(
                perst_indeces[i].row,
                1,
                false,
                perst_indeces[i].parent
                )
        }
        // select the first remaining playlist, if any
        if (theSessionData.rowCount(theSessionData.index(0,0))) {
            sessionSelectionModel.setCurrentIndex(
                theSessionData.index(0,0,theSessionData.index(0,0)),
                ItemSelectionModel.ClearAndSelect
                )
        }

        // This is a little iffy, but we have problems if user deletes the parent
        // playlist of a timeline that is being viewed because qml tries to
        // get data from the timeline after it has been deleted .. adding a
        // 200ms delay gets around this as the 'sessionSelectionModel.clear()'
        // above should disconnect the UI from the timeline
        /*callbackTimer.setTimeout(function(perst_indeces) { return function() {
            for (var i = 0; i < perst_indeces.length; ++i) {
                theSessionData.removeRows(
                    perst_indeces[i].row,
                    1,
                    false,
                    perst_indeces[i].parent
                    )
            }
            // select the first remaining playlist, if any
            if (theSessionData.rowCount(theSessionData.index(0,0))) {
                sessionSelectionModel.setCurrentIndex(
                    theSessionData.index(0,0,theSessionData.index(0,0)),
                    ItemSelectionModel.ClearAndSelect
                    )
            }
        }}(perst_indeces), 200);*/


        /**/

    }

    function showContextMenu(mx, my, parentWidget) {
        if (context_menu_loader.item == undefined) {
            context_menu_loader.sourceComponent = contextMenuComponent
        }
        context_menu_loader.item.visible = true
        repositionPopupMenu(
            context_menu_loader.item,
            parentWidget,
            mx,
            my);
    }


}