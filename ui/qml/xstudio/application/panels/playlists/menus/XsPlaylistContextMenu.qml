// SPDX-License-Identifier: Apache-2.0
import QtQuick

import xStudio 1.0
import xstudio.qml.models 1.0
import xstudio.qml.clipboard 1.0
import xstudio.qml.viewport 1.0
import "."

XsPopupMenu {

    id: contextMenu
    visible: false
    menu_model_name: "playlist_context_menu"

    Clipboard {
      id: clipboard
    }

    XsPlaylistPlusMenu {
        menu_model_name: "playlist_context_menu"
        path: "Add New"
    }

    // XsMenuModelItem {
    //     menuItemType: "divider"
    //     menuPath: ""
    //     menuItemPosition: -1.0
    //     menuModelName: contextMenu.menu_model_name
    // }

    Component.onCompleted: {
        // make sure the 'Add' sub-menu appears in the correct place
        helpers.setMenuPathPosition("Add New", "playlist_context_menu", -2.0)
        helpers.setMenuPathPosition("Copy To Clipboard", "playlist_context_menu", 0.8)
        helpers.setMenuPathPosition("Cleanup", "playlist_context_menu", 22.0)
        helpers.setMenuPathPosition("Export", "playlist_context_menu", 4.0)
        // need to reorder snippet menus..
        let rc = embeddedPython.playlistMenuModel.rowCount();
        for(let i=0; i < embeddedPython.playlistMenuModel.rowCount(); i++) {
            let fi = embeddedPython.playlistMenuModel.index(i, 0)
            let si = embeddedPython.playlistMenuModel.mapToSource(fi)
            let mp = si.model.get(si, "menuPathRole")
            helpers.setMenuPathPosition(mp,"playlist_context_menu", 9 + ((1.0/rc)*i) )
        }
    }

    // property idenfies the 'panel' that is the anticedent of this
    // menu instance. As this menu is instanced multiple times in the
    // xstudio interface we use this context property to ensure our
    // 'onActivated' callback/signal is only triggered in the corresponding
    // XsMenuModelItem instance.
    property var panelContext: helpers.contextPanel(contextMenu)

    XsFlagMenuInserter {
        text: "Set Colour"
        panelContext: contextMenu.panelContext
        menuModelName: contextMenu.menu_model_name
        menuPath: ""
        menuPosition: 0.0
        onFlagSet: (flag, flag_text) => {
            for (var i = 0; i < playlistsSelectionModel.selectedIndexes.length; ++i) {
                let index = playlistsSelectionModel.selectedIndexes[i]
                playlistsModelData.set(index, flag, "flagColourRole")
                if (flag_text)
                    playlistsModelData.set(index, flag_text, "flagTextRole")
            }
        }
    }

    XsMenuModelItem {
        text: "Selected Names"
        menuItemType: "button"
        menuPath: "Copy To Clipboard"
        menuItemPosition: 2.0
        menuModelName: contextMenu.menu_model_name
        onActivated: playlist_functions.copySelectedNames()

        hotkeyUuid: hotkey_area.copy_names_to_clipboard_hotkey.uuid

        panelContext: contextMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 0.9
        menuModelName: contextMenu.menu_model_name
    }

    XsMenuModelItem {
        text: "Rename ..."
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 1.0
        menuModelName: contextMenu.menu_model_name
        onActivated: playlist_functions.renamePlaylist()
        hotkeyUuid: hotkey_area.rename_hotkey.uuid
        panelContext: contextMenu.panelContext
    }

    XsMenuModelItem {
        text: "Duplicate"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 2.0
        menuModelName: contextMenu.menu_model_name
        onActivated: playlist_functions.duplicate()
        hotkeyUuid: hotkey_area.duplicate_hotkey.uuid
        panelContext: contextMenu.panelContext
    }

    XsMenuModelItem {
        text: "Combine"
        panelContext: contextMenu.panelContext
        menuModelName: contextMenu.menu_model_name
        menuPath: ""
        hotkeyUuid: hotkey_area.combine_hotkey.uuid
        menuItemPosition: 3.0
        onActivated: playlist_functions.combine()
    }

    XsMenuModelItem {
        text: "Selected Playlists..."
        panelContext: contextMenu.panelContext
        menuModelName: contextMenu.menu_model_name
        menuPath: "Export"
        menuItemPosition: 4.0
        onActivated: file_functions.saveSelectionNewPath(undefined)
    }

    XsMenuModelItem {
        text: "Selected Sequence as OTIO ..."
        menuItemPosition: 4.5
        panelContext: contextMenu.panelContext
        menuModelName: contextMenu.menu_model_name
        menuPath: "Export"
        onActivated: file_functions.exportSequencePath(function(result){if(result) {dialogHelpers.errorDialogFunc("Export Sequence Succeeded", "OTIO Exported")} else {dialogHelpers.errorDialogFunc("Export Sequence Failed", result)} })
    }

    XsMenuModelItem {
        text: "Snippet"
        menuItemType: "divider"
        menuItemPosition: 8
        menuPath: ""
        menuModelName: contextMenu.menu_model_name
    }

    Repeater {
        model: DelegateModel {
            model: embeddedPython.playlistMenuModel
            delegate: Item {
                XsHotkey {
                    id: hk
                    name: nameRole
                    description: nameRole
                    componentName: "Playlist Snippets"
                    onActivated: embeddedPython.pyEvalFile(scriptPathRole)
                    sequence: menuHotKeyRole || null
                    context: ""+contextMenu.panelContext
                }

                XsMenuModelItem {
                    hotkeyUuid: hk.uuid
                    text: nameRole
                    menuPath: menuPathRole
                    menuItemPosition: (index*0.01)+8
                    menuModelName: contextMenu.menu_model_name
                    onActivated: embeddedPython.pyEvalFile(scriptPathRole)
                    panelContext: contextMenu.panelContext
                }
            }
        }
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 20
        menuModelName: contextMenu.menu_model_name
    }

    XsMenuModelItem {
        text: "Remove Selected"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 21
        menuModelName: contextMenu.menu_model_name
        onActivated: removeSelected()
        panelContext: contextMenu.panelContext
        hotkeyUuid: hotkey_area.remove_selected.uuid
    }

    XsMenuModelItem {
        text: "Remove Unused Media"
        menuItemType: "button"
        menuPath: "Cleanup"
        menuItemPosition: 32
        menuModelName: contextMenu.menu_model_name
        onActivated: playlist_functions.removeUnusedMedia()
        panelContext: contextMenu.panelContext
        hotkeyUuid: hotkey_area.remove_unused_hotkey.uuid
    }
}
