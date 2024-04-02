// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12

import xStudioReskin 1.0
import xstudio.qml.models 1.0

XsPopupMenu {

    id: plusMenu
    visible: false
    menu_model_name: "playlsit_plus_button_menu"

    XsFileFunctions {
        id: file_functions
    }

    // property idenfies the 'panel' that is the anticedent of this
    // menu instance. As this menu is instanced multiple times in the
    // xstudio interface we use this context property to ensure our
    // 'onActivated' callback/signal is only triggered in the corresponding
    // XsMenuModelItem instance.
    property var panelContext: helpers.contextPanel(plusMenu)

    XsMenuModelItem {
        text: "Add Playlist"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 1
        menuModelName: plusMenu.menu_model_name
        onActivated: {
            dialogHelpers.textInputDialog(
                plusMenu.addPlaylist,
                "Enter New Playlist Name",
                "Enter a name for the new playlist.",
                "New Playlist",
                ["Ok", "Cancel"])
        }
        panelContext: plusMenu.panelContext
    }
        
    XsMenuModelItem {
        text: "Add Subset"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: plusMenu.menu_model_name
        onActivated: {
            dialogHelpers.textInputDialog(
                plusMenu.addSubset,
                "Enter New Subset Name",
                "Enter a name for the new subset.",
                "New Subset",
                ["Ok", "Cancel"])
        }
        panelContext: plusMenu.panelContext
    }

    XsMenuModelItem {
        text: "Add Timeline"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: plusMenu.menu_model_name
        onActivated: {
            dialogHelpers.textInputDialog(
                plusMenu.addTimeline,
                "Enter New Timline Name",
                "Enter a name for the new timline.",
                "New Timline",
                ["Ok", "Cancel"])
        }
        panelContext: plusMenu.panelContext
    }

    XsMenuModelItem {
        text: "Add Contact Sheet"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: plusMenu.menu_model_name
        enabled: false
        onActivated: {
            
        }
        panelContext: plusMenu.panelContext
    }

    XsMenuModelItem {
        text: "Add Group Divider"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 3.5
        menuModelName: plusMenu.menu_model_name
        onActivated: {
            dialogHelpers.textInputDialog(
                plusMenu.addDivider,
                "Enter New Divider Name",
                "Enter a name for the new divider.",
                "New Divider",
                ["Ok", "Cancel"])
        }
        panelContext: plusMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 4
        menuModelName: plusMenu.menu_model_name
    }

    XsMenuModelItem {
        text: "Add Media ..."
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 5
        menuModelName: plusMenu.menu_model_name
        onActivated: {
            file_functions.loadMedia(undefined)
        }
        panelContext: plusMenu.panelContext
    }

    function addPlaylist(new_name, button) {
        if (button == "Ok") {
            theSessionData.createPlaylist(new_name)
        }
    }

    function addSubset(new_name, button) {
        if (button == "Ok") {
            theSessionData.createSubItem(new_name, "Subset")
        }
    }

    function addTimeline(new_name, button) {
        if (button == "Ok") {
            theSessionData.createSubItem(new_name, "Timeline")
        }
    }

    function addDivider(new_name, button) {
        if (button == "Ok") {
            theSessionData.createDivider(new_name)
        }
    }

}
