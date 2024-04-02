import QtQuick 2.12
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

Item {

    id: hud
    anchors.fill: parent

    // here we access attribute data that declares a QML item for drawing
    // overlays into the viewport. Attributes that are added to the 
    // 'viewport_overlay_plugins' attribute group and that have 'qml_code'
    // role data are instatiated here

    XsModuleData {
        id: viewport_overlays
        modelDataName: "viewport_overlay_plugins"
    }

    Repeater {

        id: viewport_overlay_plugins
        anchors.fill: parent
        model: viewport_overlays

        delegate: Item {

            id: parent_item
            anchors.fill: parent

            property var dynamic_widget

            property var type_: type ? type : null

            onType_Changed: {
                if (type == "QmlCode") {
                    dynamic_widget = Qt.createQmlObject(qml_code, parent_item)
                }
            }
        }
    }

    XsModuleData {
        id: hud_elements_bottom_left
        modelDataName: "hud_elements_bottom_left"
    }

    XsModuleData {
        id: hud_elements_bottom_center
        modelDataName: "hud_elements_bottom_center"
    }

    XsModuleData {
        id: hud_elements_bottom_right
        modelDataName: "hud_elements_bottom_right"
    }

    XsModuleData {
        id: hud_elements_top_left
        modelDataName: "hud_elements_top_left"
    }

    XsModuleData {
        id: hud_elements_top_center
        modelDataName: "hud_elements_top_center"
    }

    XsModuleData {
        id: hud_elements_top_right
        modelDataName: "hud_elements_top_right"
    }

    // connect to the viewport toolbar data
    XsModuleData {        
        id: toolbar_data
        modelDataName: view.name + "_toolbar"
    }
    
    XsAttributeValue {
        id: hud_enabled_prop
        attributeTitle: "HUD"
        model: toolbar_data
    }
    visible: hud_enabled_prop.value != undefined ? hud_enabled_prop.value : false

    property var hud_margin: 10

    Column {

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: hud.hud_margin
        Repeater {

            model: hud_elements_bottom_left

            delegate: Item {

                id: parent_item
                width: dynamic_widget.width
                height: dynamic_widget.height

                property var dynamic_widget

                property var type_: type ? type : null

                onType_Changed: {
                    if (type == "QmlCode") {
                        dynamic_widget = Qt.createQmlObject(qml_code, parent_item)
                    }
                }
            }
        }
    }

    Column {

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: hud.hud_margin
        Repeater {

            model: hud_elements_bottom_center

            delegate: Item {

                id: parent_item
                width: dynamic_widget.width
                height: dynamic_widget.height

                property var dynamic_widget

                property var type_: type ? type : null

                onType_Changed: {
                    if (type == "QmlCode") {
                        dynamic_widget = Qt.createQmlObject(qml_code, parent_item)
                    }
                }
            }
        }
    }

    Column {

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: hud.hud_margin
        Repeater {

            model: hud_elements_bottom_right

            delegate: Item {

                id: parent_item
                width: dynamic_widget.width
                height: dynamic_widget.height

                property var dynamic_widget

                property var type_: type ? type : null

                onType_Changed: {
                    if (type == "QmlCode") {
                        dynamic_widget = Qt.createQmlObject(qml_code, parent_item)
                    }
                }
            }
        }
    }

    Column {

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: hud.hud_margin
        Repeater {

            model: hud_elements_top_left

            delegate: Item {

                id: parent_item
                width: dynamic_widget.width
                height: dynamic_widget.height

                property var dynamic_widget

                property var type_: type ? type : null

                onType_Changed: {
                    if (type == "QmlCode") {
                        dynamic_widget = Qt.createQmlObject(qml_code, parent_item)
                    }
                }
            }
        }
    }

    Column {

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: hud.hud_margin
        Repeater {

            model: hud_elements_top_center

            delegate: Item {

                id: parent_item
                width: dynamic_widget.width
                height: dynamic_widget.height

                property var dynamic_widget

                property var type_: type ? type : null

                onType_Changed: {
                    if (type == "QmlCode") {
                        dynamic_widget = Qt.createQmlObject(qml_code, parent_item)
                    }
                }
            }
        }
    }

    Column {

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: hud.hud_margin
        Repeater {

            model: hud_elements_top_right

            delegate: Item {

                id: parent_item
                width: dynamic_widget.width
                height: dynamic_widget.height

                property var dynamic_widget

                property var type_: type ? type : null

                onType_Changed: {
                    if (type == "QmlCode") {
                        dynamic_widget = Qt.createQmlObject(qml_code, parent_item)
                    }
                }
            }
        }
    }
}
