// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import Qt.labs.qmlmodels 1.0
import QtQml.Models 2.14
import QuickFuture 1.0
import QuickPromise 1.0

import xStudioReskin 1.0
import xstudio.qml.module 1.0
import xstudio.qml.conform 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.viewport 1.0


Item{

	XsConformEngine	{
		id: engine
		onJsonChanged: {
			replaceModel.notifyModel = engine
			replaceModel.rootIndex = engine.index(-1,-1)
			compareModel.notifyModel = engine
			compareModel.rootIndex = engine.index(-1,-1)
		}
	}

    XsHotkey {
        id: next_version_hotkey
        sequence: "Alt+n"
        name: "Next Version"
        description: "Replace With Next Version"
        // context: "" + mediaList
        // onActivated: {
        //     selectAll()
        // }
    }
    XsHotkey {
        id: previous_version_hotkey
        sequence: "Alt+p"
        name: "Previous Version"
        description: "Replace With Previous Version"
        // context: "" + mediaList
        // onActivated: {
        //     selectAll()
        // }
    }
    XsHotkey {
        id: latest_version_hotkey
        sequence: "Alt+l"
        name: "Latest Version"
        description: "Replace With Latest Version"
        // context: "" + mediaList
        // onActivated: {
        //     selectAll()
        // }
    }

    function replaceSelection(task, selection) {
        for(let i=0; i< selection.length; i++) {
            Future.promise(
                engine.conformMediaFuture(task, selection[i].model.getPlaylistIndex(selection[i]), selection[i], true)
            ).then(
                function(media_uuid_list) {
                    // console.log(media_uuid_list)
                },
                function() {
                }
            )
        }
    }

    function replaceSelectionTimeline(task, selection) {
        for(let i=0; i< selection.length; i++) {
            Future.promise(
                engine.conformMediaFuture(task, selection[i].model.getPlaylistIndex(selection[i]), selection[i], false)
            ).then(
                function(media_uuid_list) {
                    // console.log(media_uuid_list)
                },
                function() {
                }
            )
        }
    }

	function compareSelection(task, selection) {
    	for(let i=0; i< selection.length; i++) {
            Future.promise(
                engine.conformMediaFuture(task, selection[i].model.getPlaylistIndex(selection[i]), selection[i], false)
            ).then(
            	function(media_uuid_list) {
                    // create new selection.
        			// console.log(media_uuid_list)

                    let tmp = []
                    for(let i=0;i<selection.length;i++)
                        tmp.push(selection[i].model.get(selection[i], "actorUuidRole"))

                    for(let i=0;i<media_uuid_list.length;i++)
                        tmp.push(helpers.QVariantFromUuidString(media_uuid_list[i]))

                    mediaSelectionModel.selectNewMedia(selection[i].model.getPlaylistIndex(selection[i]), tmp)

            	},
            	function() {
            	}
            )
    	}
	}

    XsMenuModelItem {
        text: "Conform"
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 30
        menuModelName: "media_list_menu_"
    }

    XsMenuModelItem {
        text: "Replace"
        // menuItemType: "button"
        menuPath: ""
        menuItemPosition: 34
        menuModelName: "media_list_menu_"
    }

    XsMenuModelItem {
        text: "Compare"
        // menuItemType: "button"
        menuPath: ""
        menuItemPosition: 35
        menuModelName: "media_list_menu_"
    }

    XsMenuModelItem {
        text: "Replace"
        menuPath: ""
        menuItemPosition: 34
        menuModelName: "timeline_menu_"
    }

	DelegateModel {
		id: replaceModel
		property var notifyModel: null
		model: notifyModel
		delegate :
			Item {
				XsMenuModelItem {
					text: nameRole
					menuItemType: "button"
					menuPath: "Replace"
					menuItemPosition: 34 + index
					menuModelName: "media_list_menu_"
			        onActivated: replaceSelection(text, menuContext.mediaSelection)
                    hotkeyUuid: {
                        if(nameRole == "Next Version")
                            return next_version_hotkey.uuid
                        else if(nameRole == "Previous Version")
                            return previous_version_hotkey.uuid
                        else if(nameRole == "Latest Version")
                            return latest_version_hotkey.uuid
                        helpers.QVariantFromUuidString("")
                    }
				}
                XsMenuModelItem {
                    text: nameRole
                    menuItemType: "button"
                    menuPath: "Replace"
                    menuItemPosition: 34 + index
                    menuModelName: "timeline_menu_"
                    onActivated: replaceSelectionTimeline(text, menuContext.theTimeline.timelineSelection.selectedIndexes)
                }
			}
	}

	DelegateModel {
		id: compareModel
		property var notifyModel: null
		model: notifyModel
		delegate :
			Item {
				XsMenuModelItem {
					text: nameRole
					menuItemType: "button"
					menuPath: "Compare"
					menuItemPosition: 35 + index
					menuModelName: "media_list_menu_"
			        onActivated: compareSelection(text, menuContext.mediaSelection)
				}
			}
	}

    Repeater {
		model: replaceModel
	}

    Repeater {
		model: compareModel
	}
}



    // my_menu_      = insert_menu_item("media_list_menu_", "Conform", "", 0.0f);
    // compare_menu_ = insert_menu_item("media_list_menu_", "Compare", "Conform", 0.0f);
    // replace_menu_ = insert_menu_item("media_list_menu_", "Replace", "Conform", 0.0f);

    // next_menu_item_ = insert_menu_item("media_list_menu_", "Next Version", "Conform", 0.0f);
    // previous_menu_item_ =
    //     insert_menu_item("media_list_menu_", "Previous Version", "Conform", 0.0f);
    // latest_menu_item_ = insert_menu_item("media_list_menu_", "Latest Version", "Conform", 0.0f);
