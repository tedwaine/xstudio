// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.12

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0
import xstudio.qml.helpers 1.0

XsPopupMenu {

    id: btnMenu
    visible: false
    menu_model_name: "media_list_menu_"

    property var panelContext: helpers.contextPanel(btnMenu)

    /**************************************************************

    Static Menu Items (most items in this menu are added dymanically
        from the backend - e.g. PlayheadActor, Viewport classes do this)

    ****************************************************************/

    XsMenuModelItem {
        text: "Select All"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        hotkeyUuid: select_all_hotkey.uuid
        onActivated: {
            mediaList.selectAll()
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "De-Select All"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: btnMenu.menu_model_name
        hotkeyUuid: deselect_all_hotkey.uuid
        onActivated: {
            mediaList.deselectAll()
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Dump Metadata"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            console.log(theSessionData.getJSON(mediaList.selection[0],""))
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Delete Selected"
        menuPath: ""
        menuItemPosition: 3.5
        menuModelName: btnMenu.menu_model_name
        hotkeyUuid: delete_selected.uuid
        onActivated: {
            deleteSelected()
        }
        panelContext: btnMenu.panelContext
    }

}
