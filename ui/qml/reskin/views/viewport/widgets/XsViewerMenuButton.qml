// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.15
import QtQml.Models 2.12

import xStudioReskin 1.0
import xstudio.qml.models 1.0

XsViewerToolbarButtonBase {
        
    id: theButton
    isActive: menu_loader.item ? menu_loader.item.visible : false
    showBorder: mouseArea.containsMouse

    MouseArea{ 
        
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        enabled: !disabled // from parent item class
        onClicked: {
            if (menu_loader.item == undefined) {
                menu_loader.sourceComponent = menuComponent
            }
            showPopupMenu(
                menu_loader.item,
                theButton,
                0,
                -menu_loader.item.height);
        }

    }

    // This menu works by picking up the 'value' and 'combo_box_options' role
    // data that is exposed via the model that instantiated this XsViewerMenuButton
    // instance
    

    Loader {
        id: menu_loader
    }

    Component {
        id: menuComponent
        XsMenuMultiChoice { 
            id: btnMenu
            visible: false
            property bool is_enabled: true
        }
    }

}