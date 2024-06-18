// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import "."

XsPopupMenu {

    id: contextMenu
    visible: false
    menu_model_name: "playlist_context_menu"

    XsPlaylistPlusMenu {
        menu_model_name: "playlist_context_menu"
        path: "Add"
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: -1.0
        menuModelName: contextMenu.menu_model_name
    }

    Component.onCompleted: {
        // make sure the 'Add' sub-menu appears in the correct place
        helpers.setMenuPathPosition("Add", "playlist_context_menu", -2.0)
    }

    // property idenfies the 'panel' that is the anticedent of this
    // menu instance. As this menu is instanced multiple times in the
    // xstudio interface we use this context property to ensure our
    // 'onActivated' callback/signal is only triggered in the corresponding
    // XsMenuModelItem instance.
    property var panelContext: helpers.contextPanel(contextMenu)

    XsFlagMenuInserter {
        text: "Media Colour"
        panelContext: contextMenu.panelContext
        menuModelName: contextMenu.menu_model_name
        menuPath: ""
        menuPosition: 0.0
        onFlagSet: {
            for (var i = 0; i < sessionSelectionModel.selectedIndexes.length; ++i) {
                let index = sessionSelectionModel.selectedIndexes[i]
                theSessionData.set(index, flag, "flagColourRole")
                if (flag_text)
                    theSessionData.set(index, flag_text, "flagTextRole")
            }            
        }
    }

    XsMenuModelItem {
        text: "Combine Selected Playlists"
        panelContext: contextMenu.panelContext
        menuModelName: contextMenu.menu_model_name
        menuPath: ""
        menuItemPosition: 1.0
        onActivated: {
            if(sessionSelectionModel.selectedIndexes.length) {
                theSessionData.mergeRows(sessionSelectionModel.selectedIndexes)
            }
        }
    }

    XsMenuModelItem {
        text: "Export Selected Playlists as Session ..."
        panelContext: contextMenu.panelContext
        menuModelName: contextMenu.menu_model_name
        menuPath: ""
        menuItemPosition: 2.0
        onActivated: {
            file_functions.saveSelelctionNewPath(undefined)
        }
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 3.0
        menuModelName: contextMenu.menu_model_name
    }

    XsMenuModelItem {
        text: "Remove Media not in Timeline/Subsets"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 4.0
        menuModelName: contextMenu.menu_model_name
        onActivated: {
            for (var idx = 0; idx < sessionSelectionModel.selectedIndexes.length; ++idx) {
                if (theSessionData.get(sessionSelectionModel.selectedIndexes[idx], "typeRole") == "Playlist") {
                    theSessionData.purgePlaylist(sessionSelectionModel.selectedIndexes[idx])
                }
            }
        }
        panelContext: contextMenu.panelContext
    }

    XsMenuModelItem {
        text: "Rename ..."
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 5.0
        menuModelName: contextMenu.menu_model_name
        property var targetIdx
        onActivated: {
            if(sessionSelectionModel.selectedIndexes.length != 1) {
                dialogHelpers.errorDialogFunc(
                    "Rename Playlist ...",
                    "Please select a single item to rename."
                    )
            } else {
                targetIdx = sessionSelectionModel.selectedIndexes[0]
                let name = theSessionData.get(targetIdx, "nameRole")
                let type = theSessionData.get(targetIdx, "typeRole")
                dialogHelpers.textInputDialog(
                    rename,
                    "Rename " + type,
                    "Enter a new name for the " + type,
                    name,
                    ["Cancel", "Rename " + type])
            }
        }
        panelContext: contextMenu.panelContext
        function rename(new_name, button) {
            if (button == "Cancel") return
            theSessionData.set(targetIdx, new_name, "nameRole")
        }
    }


    XsMenuModelItem {
        text: "Duplicate Selected"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 6.0
        menuModelName: contextMenu.menu_model_name
        onActivated: {
            for (var i = 0; i < sessionSelectionModel.selectedIndexes.length; ++i) {
                let index = sessionSelectionModel.selectedIndexes[i]
                theSessionData.duplicateRows(index.row, 1, index.parent)
            }
        }
        panelContext: contextMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 6.5
        menuModelName: contextMenu.menu_model_name
    }

    XsMenuModelItem {
        text: "Remove Selected"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 7.0
        menuModelName: contextMenu.menu_model_name
        onActivated: removeSelected()
        panelContext: contextMenu.panelContext
    }
}
