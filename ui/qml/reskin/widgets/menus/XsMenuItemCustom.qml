import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.3

import xStudioReskin 1.0
import xstudio.qml.models 1.0

Item {

    id: widget            

    property var dynamic_widget
    property real minWidth: dynamic_widget ? dynamic_widget.minWidth : 0
    height: dynamic_widget ? dynamic_widget.height : 0

    property var custom_menu_qml_: custom_menu_qml ? custom_menu_qml : null

    onCustom_menu_qml_Changed: {
        dynamic_widget = Qt.createQmlObject(custom_menu_qml_, widget)
    }

    property real leftIconSize: 0

    property var parent_menu
    property var menu_model
    property var menu_model_index
    
    function hideSubMenus() {}

    MouseArea
    { 
        id: menuMouseArea
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true                    
    }
}