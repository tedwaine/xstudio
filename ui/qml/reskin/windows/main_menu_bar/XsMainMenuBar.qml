import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0

import "./file_menu/"

XsMenuBar {

    id: menu_bar
    height: XsStyleSheet.menuHeight

    menu_model_name: "main menu bar"

    XsFileMenu {}

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: "Panels"
        menuModelName: "main menu bar"
        Component.onCompleted: {
            setMenuPathPosition("Color", 20.0)
            setMenuPathPosition("Help", 21.0)
        }    
    }


    XsMenuModelItem {
        menuItemType: "toggle"
        text: "UI Flat Theme"
        menuPath: "Panels|Settings"
        menuModelName: "main menu bar"

        // onIsCheckedChanged: {
        //     if (isChecked) {
        //         gradTimer.running = true
        //     }
        //     else{
        //         gradTimer.running = false
        //     }
        // }

        isChecked: appWindow.isFlatTheme
        onActivated: {
            appWindow.isFlatTheme = !appWindow.isFlatTheme
        }
    }

    Repeater {
        model: accentColorModel
        Item {
            XsMenuModelItem {
                text: name
                menuPath: "Panels|Settings|UI Accent Colour"
                menuModelName: "main menu bar"

                // declare this as a custom menu item
                menuItemType: "custom"

                // puts the colour we want into userData property of the menu
                // model item - the user data is stored in the model and can
                // be accessed with the ui element is intantiated as 'user_data'
                // context data
                userData: value

                // here we can create any widget we like that gets inserted
                // into the pop-up menu. It muse have a 'minWidth' property
                // which gives the width of the contents.
                customMenuQml: `
                    import QtQuick 2.15
                    import QtQuick.Layouts 1.3
                    import xStudioReskin 1.0
                    Rectangle {
                        width: parent.width
                        height: XsStyleSheet.menuHeight
                        property var minWidth: metrics.width + 20
                        color: user_data // here we retrieve the colour from user_data
                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked: {
                                XsStyleSheet.accentColor = user_data
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 4
                            Rectangle {
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                color: user_data == XsStyleSheet.accentColor ? "black" : "transparent"
                                width: 7
                                height: 7
                                radius: 3.5
                            }
                            Text {
                                id: labelDiv
                                text: name ? name : "Unknown" //+ (sub_menu && !is_in_bar ? "   >>" : "")
                                font.pixelSize: XsStyleSheet.fontSize
                                font.family: XsStyleSheet.fontFamily
                                color: "black"
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                Layout.fillWidth: true
                            }
                            TextMetrics {
                                id:     metrics
                                font:   labelDiv.font
                                text:   labelDiv.text
                            }

                        }
                    }
                    `
                onActivated: {
                    XsStyleSheet.accentColor = value
                }
            }
        }
    }

    ListModel { id: accentColorModel

        ListElement {
            name: qsTr("Blue")
            value: "#307bf6"
        }
        ListElement {
            name: qsTr("Purple")
            value: "#9b56a3"
        }
        ListElement {
            name: qsTr("Pink")
            value: "#e65d9c"
        }
        ListElement {
            name: qsTr("Red")
            value: "#ed5f5d"
        }
        ListElement {
            name: qsTr("Orange")
            value: "#e9883a"
        }
        ListElement {
            name: qsTr("Yellow")
            value: "#f3ba4b"
        }
        ListElement {
            name: qsTr("Green")
            value: "#77b756"
        }
        ListElement {
            name: qsTr("Graphite")
            value: "#999999"//"#666666"
        }

    }

}