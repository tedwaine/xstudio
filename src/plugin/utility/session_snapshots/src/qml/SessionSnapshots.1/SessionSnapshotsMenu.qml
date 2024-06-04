// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQml 2.14
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QuickFuture 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import SessionSnapshots 1.0

import "."

XsMenuItemNew {

	is_in_bar: true
	id: snapshotMenu

	property var menuContextData: undefined
	property var menu_item_enabled: true
	property var menu_icon: undefined
	property var name: "Snapshots"
	property var hotkey_sequence: undefined
	property var parent_menu: menu_bar
	property var action_idx

	function saveSnapshot(snapshot_name, button) {
            
		if (button != "Save Snapshot") return
		let path = snapshotModel.buildSavePath(action_idx, snapshot_name)		
		console.log("saveSnapshot", path)
		file_functions.saveSessionAs(path, undefined, doRescan)

	}

	function doRescan() {
		if (action_idx && action_idx.valid) snapshotModel.rescan(action_idx, 1);
	}

	function newFolder(folder_name, button) {
		if (button != "Create Folder") return
		snapshotModel.createFolder(action_idx, folder_name)
		doRescan()
	}

	XsModelProperty {
        id: __snapshot_paths
        role: "valueRole"
        index: globalStoreModel.searchRecursive("/core/snapshot/paths", "pathRole")
    }
	property alias snapshot_paths: __snapshot_paths.value

    SessionSnapshotsMenuModel {
    	id: snapshotModel
    	paths: snapshot_paths
		onJsonChanged: {
			menu_model_index = snapshotModel.index(-1, -1)
		}

		// we provide this function which is called by certain instances of
		// XsMenuItem within the sub menus that we are dynamically building
		// to show the filesystem tree in these menus.
		// When a sub menu becomes visible, we get the XsSnapshotMenuModel to 
		// rescan the filesystem folder whose content is shown in the given 
		// sub-menu.
		function menuItemVisibilityChanged(index, visibility, panel) {
			if (visibility) {
				rescan(index, 1);
			}
		}

		function nodeActivated(index, data, panel) {

			var action_type = get(index, "user_data")
			var path = get(index, "snapshot_filesystem_path")
			var name = get(index, "name")
			action_idx = index.parent

			if (action_type == "SNAPSHOT_REMOVE_SNAPSHOT") {

				let v = snapshot_paths
				let new_v = []	
				for(let i =0; i< v.length;i++) {
					if(path != v[i].path)
						new_v.push(v[i])
				}
				snapshot_paths = new_v

			} else if (action_type == "SNAPSHOT_ADD") {

				loader.sourceComponent = new_snapshot_folder_dlg
				loader.item.visible = true	

			} else if (action_type == "SNAPSHOT_SAVE") {

				dialogHelpers.textInputDialog(
					saveSnapshot,
					"Save Snapshot",
					"Enter a name for your session snapshot.",
					"",
					["Cancel", "Save Snapshot"])
				
			} else if (action_type == "SNAPSHOT_NEW_FOLDER") {

				dialogHelpers.textInputDialog(
					newFolder,
					"Create a New Folder",
					"Enter a name for a new snapshot sub-folder",
					"",
					["Cancel", "Create Folder"])
				
			} else if (action_type == "SNAPSHOT_LOAD") {

				Future.promise(theSessionData.importFuture(path, null)).then(
					function(result) {
						if (result) {
							dialogHelpers.errorDialogFunc("Snapshot Loaded", "Snapshot " + name + " was added to your session.")
						} else {
							dialogHelpers.errorDialogFunc("Snapshot ERror", "Snapshot " + name + " was not added to your session, an error occurred. Check your terminal for more info.")
						}
					})
				
			}
		}
    }

	menu_model: snapshotModel
    menu_model_index: snapshotModel.index(-1, -1)

	Component {		
		id: new_snapshot_folder_dlg
		NewSnapshotDialog {
			id: snapshot_path_dialog
			onVisibilityChanged: {
				if (!visible) {
					// not sure if this actually helps, but assuming
					// that 'unloading' the dialog destroys it
					loader.sourceComponent = undefined
				}
			}
		}	
	}

	Loader {
		id: loader
	}
}

/*XsMenuNew {

    id: snapshotMenu

	XsModelProperty {
        id: __snapshot_paths
        role: "valueRole"
        index: globalStoreModel.searchRecursive("/core/snapshot/paths", "pathRole")
    }
	property alias snapshot_paths: __snapshot_paths.value

    XsSnapshotMenuModel {
    	id: snapshotModel
    	paths: snapshot_paths
    }

	menu_model: snapshotModel
    menu_model_index: snapshotModel.index(-1, -1)

	

    ListView {
        id: snapshot_items
        model: snapshotModelDelegate
    }

	DelegateModel {
		id: snapshotModelDelegate
		model: snapshotModel
		delegate: XsSnapshotMenuItem {
			menuPath: "Snapshots"
			rootIndex: snapshotModel.index(index, 0)
			position: index
			top_level: true
		}
	}*/

	/*XsMenuModelItem {
        menuItemType: "divider"
        menuPath: "Snapshots"
        menuItemPosition: 99.0
        menuModelName: "main menu bar"
    }

	XsMenuModelItem {
        text: "Add Snapshot Folder ..."
        menuPath: "Snapshots"
        menuItemPosition: 100.0
        menuModelName: "main menu bar"
        onActivated: {
            loader.sourceComponent = new_snapshot_folder_dlg
			loader.item.visible = true
        }
    }
}*/
