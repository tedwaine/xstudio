// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3
import QtGraphicalEffects 1.15

import xStudioReskin 1.0
import xstudio.qml.module 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.viewport 1.0

import ShotBrowser 1.0

Item
{
    id: shotbrowserPanel
    anchors.fill: parent
    

    XsGradientRectangle{ 
        id: backgroundDiv
        anchors.fill: parent
    }

    // using the ID in this string makes a unique string for each instance
    // of the ShotBrowserPanel, so it has its own private menu model from
    // which it can build actual menus. This menuModelName has to be used
    // with any XsMenuItem {} you declare to insert menu items into the menu
    // model.
    // Note that you can have as many menu models as you like
    property string menu1ModelName: "shotbrowser_menu_1" + shotbrowserPanel

    // This item contains a bunch of XsMenuItem declarations to build a
    // menu model .. but you can plonk XsMenuItem anywhere you want, you just
    // need to know the 'menuModelName' string to determine which menu model
    // it gets inserted into
    ShotBrowserMenuModel {
        menu_model_name: menu1ModelName
    }

    // Hotkeys can be declared like this
    XsHotkey {
        // 'context' can be used to filter on what had focus when the hotkey
        // was pressed, (though this mechanism needs a bit more work)
        // context: "ShotBrowser Panel"
        sequence:  "End"
        name: "To End"
        description: "Jumps to the end frame"
        onActivated: {
            console.log("END key was pressed")
        }
    }

    XsHotkey {
        // context: "ShotBrowser Panel"
        sequence: "Ctrl+Q"
        name: "Select Latest Version or Whatever"
        description: "Does some magic"
        onActivated: {
            console.log("Ctrl + Q key was pressed")
        }
    }

    ColumnLayout {

        anchors.fill: parent

        // build a menu bar from our menu model
        XsMenuBar {
            Layout.fillWidth: true
            id: menu_bar
            height: XsStyleSheet.menuHeight
            menu_model_name: menu1ModelName
        }

        Rectangle {
            color: "transparent"
            Layout.fillWidth: true
            Layout.fillHeight: true

            // build a pop-up menu from our menu model
            XsPopupMenu {
                id: popup_menu
                menu_model_name: menu1ModelName
                visible: false
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    popup_menu.visible = true
                    popup_menu.x = mouseX
                    popup_menu.y = mouseY
                }
            }

            Text {
                anchors.centerIn: parent
                text: "ShotBrowser!"
                rotation: 30
                color: "white"
                font.pixelSize: 70
                font.family: XsStyleSheet.fontFamily
                opacity: 0.7
            }

            Text {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                text: "This panel just demonstrates how to build static & dynamic menus\n\nRight Click for a Pop-up Menu"
                color: "white"
                font.pixelSize: 20
                font.family: XsStyleSheet.fontFamily
                opacity: 0.7
            }

            // demo using our plugin local icon
            Image {
                anchors.centerIn: parent
                source: "qrc:///shotbrowser_icons/shotgun.png"
                fillMode: Image.PreserveAspectFit
                width: 400
                height: 400
                layer {
                    enabled: true
                    effect:
                    ColorOverlay {
                        color: "white"
                    }
                }
                opacity: 0.3
                z: -50
            }
        }
    }

}