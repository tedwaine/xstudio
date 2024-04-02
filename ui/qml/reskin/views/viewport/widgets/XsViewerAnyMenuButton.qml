// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.12

import xStudioReskin 1.0
import xstudio.qml.models 1.0

XsViewerToolbarButtonBase {
        
    property var menuModelName
    isActive: loader.item ? loader.item.visible : false
    showBorder: mouseArea.containsMouse
    id: theButton

    // we use a loader so that pop-up menus are only created when we need 
    // to show them.

    MouseArea{ 
        
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if (!loader.item) {
                loader.sourceComponent = menuComponent
            }
            showPopupMenu(
                loader.item,
                theButton,
                0,
                -loader.item.height)
        }

    }

    Loader {
        id: loader
    }

    // This menu is built from a menu model that is maintained by xSTUDIO's
    // backend. We access the menu model by an id string 'menuModelName' that
    // will be set by the derived type
    Component {
        id: menuComponent
        XsPopupMenu { 
            id: btnMenu
            visible: true
            menu_model_name: menuModelName        
            onVisibleChanged: {
                if (!visible) {
                    loader.sourceComponent = undefined
                }
            }

        }
    }

}
