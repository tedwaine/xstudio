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
                engine.conformItemsFuture(
                    task,
                    selection[i].model.getContainerIndex(selection[i]),
                    selection[i], true, true)
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
                engine.conformItemsFuture(task,
                    selection[i].model.getContainerIndex(selection[i]),
                    selection[i], true, false)
            ).then(
                function(media_uuid_list) {
                    // console.log(media_uuid_list)
                },
                function() {
                }
            )
        }
    }

    function autoConformSelectionTimeline(task, src, dst) {
        // purge dst and clone src into it.
        if(theSessionData.replaceTimelineTrack(src, dst)){
            dst.model.set(dst, task, "nameRole")
            Future.promise(
                engine.conformItemsFuture(task,
                    dst.model.getContainerIndex(dst),
                    dst, true, true)
            ).then(
                function(media_uuid_list) {
                    // console.log(media_uuid_list)
                },
                function() {
                }
            )
        }
    }

    function conformSelectionTimeline(task, selection) {
        let clips = theSessionData.duplicateTimelineClips(selection, task, "");
        for(let i=0; i< clips.length; i++) {
            Future.promise(
                engine.conformItemsFuture(task,
                    clips[i].model.getContainerIndex(clips[i]),
                    clips[i], true, true)
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
                engine.conformItemsFuture(task,
                    selection[i].model.getContainerIndex(selection[i]),
                    selection[i], true, false)
            ).then(
            	function(media_uuid_list) {
                    // create new selection.
        			// console.log(media_uuid_list)

                    let tmp = []
                    for(let i=0;i<selection.length;i++)
                        tmp.push(selection[i].model.get(selection[i], "actorUuidRole"))

                    for(let i=0;i<media_uuid_list.length;i++)
                        tmp.push(helpers.QVariantFromUuidString(media_uuid_list[i]))

                    mediaSelectionModel.selectNewMedia(selection[i].model.getContainerIndex(selection[i]), tmp)

            	},
            	function() {
            	}
            )
    	}
	}

    function replaceToSequence(selection, sequenceIndex, conformTrackIndex=engine.index(-1,-1)) {
        if(selection.length && sequenceIndex.valid && sequenceIndex.model.get(sequenceIndex, "typeRole") == "Timeline") {
            Future.promise(
                engine.conformToSequenceFuture(
                    selection[0].model.getPlaylistIndex(selection[0]),
                    selection,
                    sequenceIndex,
                    conformTrackIndex,
                    true)
            ).then(
                function(media_uuid_list) {
                    // create new selection.
                    // console.log(media_uuid_list)
                },
                function() {
                }
            )
        }
    }

    function conformToSequence(selection, sequenceIndex, trackName="", conformTrackIndex=engine.index(-1,-1)) {
        if(selection.length && sequenceIndex.valid && sequenceIndex.model.get(sequenceIndex, "typeRole") == "Timeline") {
            Future.promise(
                engine.conformToSequenceFuture(
                    selection[0].model.getPlaylistIndex(selection[0]),
                    selection,
                    sequenceIndex,
                    conformTrackIndex,
                    false,
                    trackName)
            ).then(
                function(media_uuid_list) {
                    // create new selection.
                    // console.log(media_uuid_list)
                },
                function() {
                }
            )
        }
    }

    function conformToNewSequence(selection, playlistIndex=helpers.qModelIndex()) {
        if(selection.length) {
            Future.promise(
                engine.conformToNewSequenceFuture(
                    selection, playlistIndex
                )
            ).then(
                function(uuid_list) {
                },
                function() {
                }
            )
        }
    }

    function conformTracksToSequence(trackIndexes, sequenceIndex) {
        if(trackIndexes.length) {
            Future.promise(
                engine.conformTracksToSequenceFuture(
                    trackIndexes, sequenceIndex
                )
            ).then(
                function(uuid_list) {
                },
                function() {
                }
            )
        }
    }

    function autoConformFromTrackName(conformSourceIndex, selectedIndexes) {
        for(let i=0;i<selectedIndexes.length;i++) {
            let type = selectedIndexes[i].model.get(selectedIndexes[i], "typeRole")
            if(["Audio Track", "Video Track"].includes(type)) {
                let task = selectedIndexes[i].model.get(selectedIndexes[i], "nameRole")
                // make sure the task is valid..
                let task_index = engine.search(task, "nameRole")
                if(task_index.valid)
                    autoConformSelectionTimeline(task, conformSourceIndex, selectedIndexes[i])
            }
        }
    }


    function conformPrepareSequence(sequenceIndex, onlyCreateConfrom=true) {
        if(sequenceIndex.valid && sequenceIndex.model.get(sequenceIndex, "typeRole") == "Timeline") {
            Future.promise(
                engine.conformPrepareSequenceFuture(
                    sequenceIndex, onlyCreateConfrom
                )
            ).then(
                function(result) {
                    // create new selection.
                    // console.log(media_uuid_list)
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
        text: "Conform To Sequence"
        // menuItemType: "button"
        menuPath: ""
        menuItemPosition: 35.5
        menuModelName: "media_list_menu_"
        onActivated: conformToSequence(menuContext.mediaSelection, viewedMediaSetIndex)
    }

    XsMenuModelItem {
        text: "Conform To New Sequence"
        // menuItemType: "button"
        menuPath: ""
        menuItemPosition: 35.6
        menuModelName: "media_list_menu_"
        onActivated: conformToNewSequence(menuContext.mediaSelection)
    }


    XsMenuModelItem {
        text: "Replace"
        menuPath: ""
        menuItemPosition: 8.5
        menuModelName: "timeline_clip_menu_"
    }

    XsMenuModelItem {
        text: "Auto-Conform"
        menuPath: ""
        menuItemPosition: 8
        menuModelName: "timeline_clip_menu_"
    }

    XsMenuModelItem {
        text: "Auto-Conform"
        menuPath: ""
        menuItemPosition: 12
        menuModelName: "timeline_track_menu_"
    }

    XsMenuModelItem {
        text: "Auto-Conform From Track Name"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 13
        menuModelName: "timeline_track_menu_"
        onActivated: autoConformFromTrackName(
            menuContext.theTimeline.conformSourceIndex, menuContext.theTimeline.timelineSelection.selectedIndexes
        )
    }


    XsMenuModelItem {
        text: "Replace Clips"
        menuPath: ""
        menuItemPosition: 14
        menuModelName: "timeline_track_menu_"
    }

    XsMenuModelItem {
        text: "Conform To Sequence"
        menuPath: ""
        menuItemPosition: 14.5
        menuModelName: "timeline_track_menu_"
        onActivated: conformTracksToSequence(menuContext.theTimeline.timelineSelection.selectedIndexes, inspectedMediaSetIndex)
    }


    XsMenuModelItem {
        text: "Create Conform Track"
        // menuItemType: "button"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: "timeline_menu_"
        onActivated: conformPrepareSequence(menuContext.theTimeline.timelineModel.rootIndex.parent)
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
                    menuPath: "Auto-Conform"
                    menuItemPosition: index
                    menuModelName: "timeline_track_menu_"
                    onActivated: autoConformSelectionTimeline(text, menuContext.theTimeline.conformSourceIndex, menuContext.theTimeline.timelineSelection.selectedIndexes[0])
                }
                XsMenuModelItem {
                    text: nameRole
                    menuItemType: "button"
                    menuPath: "Replace Clips"
                    menuItemPosition: index
                    menuModelName: "timeline_track_menu_"
                    onActivated: replaceSelectionTimeline(text, menuContext.theTimeline.timelineSelection.selectedIndexes)
                }

                XsMenuModelItem {
                    text: nameRole
                    menuItemType: "button"
                    menuPath: "Replace"
                    menuItemPosition: index
                    menuModelName: "timeline_clip_menu_"
                    onActivated: replaceSelectionTimeline(text, menuContext.theTimeline.timelineSelection.selectedIndexes)
                }

                XsMenuModelItem {
                    text: nameRole
                    menuItemType: "button"
                    menuPath: "Auto-Conform"
                    menuItemPosition: index
                    menuModelName: "timeline_clip_menu_"
                    onActivated: conformSelectionTimeline(text, menuContext.theTimeline.timelineSelection.selectedIndexes)
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
					menuItemPosition: index
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
