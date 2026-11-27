import QtQuick
import QuickFuture 1.0
import QuickPromise 1.0

import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0
import xstudio.qml.clipboard 1.0
import xStudio 1.0

Item {


    /**************************************************************

    COMMON FUNCTIONS

    ****************************************************************/

    function loadMedia(media_urls) {

        let py_args = {'media_urls': []}
        media_urls.forEach(function(item) {
            py_args['media_urls'].push(String(item))
        })
        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "load_media", py_args)
        ).then(function(result) {
        },
        function(__rr) {
            console.log("ERROR", __rr)
        })

    }

    function createPlaylist(name, select=true) {

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "create_playlist", {'name': name, 'select': select})
        ).then(function(result) {
        },
        function(__rr) {
            console.log("ERROR", __rr)
        })
    }

    function handleDragDrop(action, drop_data, target_session_object) {

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "handle_drag_drop", {'action': action, 'drop_data': drop_data, 'target_session_object': target_session_object})
        ).then(function(result) {
        },
        function(__rr) {
            dialogHelpers.errorDialogFunc(
                "Handle Drag Drop Error",
                "An error occurred while handling the drag and drop operation: " + __rr
            )
        })

    }

    function rescanMedia(media_uuids) {

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "rescan_media", {'media_uuids': media_uuids})
        ).then(function(result) {
        },
        function(__rr) {
            dialogHelpers.errorDialogFunc(
                "Rescan Media Error",
                "An error occurred while rescanning media: " + __rr
            )
        })

    }

    function invalidateCache(media_uuids) {

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "invalidate_cache", {'media_uuids': media_uuids})
        ).then(function(result) {
        },
        function(__rr) {
            dialogHelpers.errorDialogFunc(
                "Invalidate Cache Error",
                "An error occurred while invalidating the cache: " + __rr
            )
        })

    }

    function decomposeMedia(container_uuid, media_uuids) {

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "decompose_media", {'container_uuid': container_uuid, 'media_uuids': media_uuids})
        ).then(function(result) {
        },
        function(__rr) {
            dialogHelpers.errorDialogFunc(
                "Decompose Media Error",
                "An error occurred while decomposing media: " + __rr
            )
        })

    }

    function sortMediaByDisplayInfoColumn(sort_column_idx, ascending) {

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "sort_media_by_display_info_column", {'sort_column_idx': sort_column_idx, 'ascending': ascending})
        ).then(function(result) {
        },
        function(__rr) {
            dialogHelpers.errorDialogFunc(
                "Sort Media Error",
                "An error occurred while sorting media: " + __rr
            )
        })

    }

    function getTimelineExportTypes() {
        return helpers.pythonCallback("UtilityPythonPlugin", "get_timeline_export_types", {})
    }

    XsPreference {
        id: sessionRate
        path: "/core/session/media_rate"
    }

    Clipboard {
      id: clipboard
    }


    function yyyymmdd(separator="", date=new Date()) {
        return String(date.getFullYear()) + separator + String(date.getMonth()+1).padStart(2, '0') + separator + String(date.getUTCDate()).padStart(2, '0')
    }

    function addDatedDivider() {
        theSessionData.createDivider(yyyymmdd("-"))
    }

    function addPlaylist(new_name, button) {
        if (button == "Add") {
            sessionFunctions.createPlaylist(new_name)
        }
    }

    function addSubset(new_name, button) {
        if (button == "Add") {
            theSessionData.createSubItem(new_name, "Subset")
        }
    }

    function addContactSheet(new_name, button) {
        if (button == "Add") {
            theSessionData.createSubItem(new_name, "ContactSheet")
        }
    }

    function addTimeline(new_name, fps, button) {
        if (button == "Add") {
            theSessionData.createSubItem(new_name, "Timeline", fps)
        }
    }

    function addDivider(new_name, button) {
        if (button == "Add") {
            theSessionData.createDivider(new_name)
        }
    }

    function renamePlaylist() {

        // FIXME
        if(playlistsSelectionModel.selectedIndexes.length != 1) {
            dialogHelpers.errorDialogFunc(
                "Rename Playlist ...",
                "Please select a single item to rename."
                )
        } else {
            let index = playlistsSelectionModel.selectedIndexes[0]
            let name = playlistsModelData.get(index, "nameRole")
            let type = playlistsModelData.get(index, "typeRole")
            dialogHelpers.textInputDialog(
                function(new_name, button) {
			        if (button == "Cancel") return
			        playlistsModelData.set(index, new_name, "nameRole")
				},
                "Rename " + type,
                "Enter a new name for the " + type,
                name,
                ["Cancel", "Rename " + type])
        }
    }

    function removeUnusedMedia() {
        // FIXME
        for (var idx = 0; idx < playlistsSelectionModel.selectedIndexes.length; ++idx) {
            if (playlistsModelData.get(playlistsSelectionModel.selectedIndexes[idx], "typeRole") == "Playlist") {
                playlistsModelData.purgePlaylist(playlistsSelectionModel.selectedIndexes[idx])
            }
        }
    }

    function duplicate() {
        // FIXME
        for (var i = 0; i < playlistsSelectionModel.selectedIndexes.length; ++i) {
            let index = playlistsSelectionModel.selectedIndexes[i]
            playlistsModelData.duplicateRows(index.row, 1, index.parent)
        }
    }

    function combine() {
        // FIXME
        if(playlistsSelectionModel.selectedIndexes.length) {
            playlistsModelData.mergeRows(playlistsSelectionModel.selectedIndexes)
        }
    }

    function copySelectedNames() {
        // FIXME
        let result = []
        for(let i =0;i<playlistsSelectionModel.selectedIndexes.length;i++) {
            result.push(playlistsModelData.get(playlistsSelectionModel.selectedIndexes[i], "nameRole"))
        }
        clipboard.text = result.join("\n")
    }

    function addPlaylistChoice() {
        dialogHelpers.textInputDialog(
            addPlaylist,
            "Add Playlist",
            "Enter a name for the new playlist.",
            the_session.getNextName("Playlist {}"),
            ["Cancel", "Add"])
    }

    function addSubsetChoice() {
        dialogHelpers.textInputDialog(
            addSubset,
            "Add Subset",
            "Enter a name for the new subset.",
            the_session.getNextName("Subset {}"),
            ["Cancel", "Add"])
    }

    function addSequenceChoice() {
        dialogHelpers.sequenceInputDialog(
            addTimeline,
            "Add Sequence",
            "New Sequence",
            the_session.getNextName("Sequence {}"),
            ["Cancel", "Add"],
            sessionRate.value)
    }

    function addContactSheetChoice() {
        dialogHelpers.textInputDialog(
            addContactSheet,
            "Add Contact Sheet",
            "Enter a name for the new contact sheet.",
            the_session.getNextName("Contact Sheet {}"),
            ["Cancel", "Add"])
    }

    function addDividerChoice() {
        dialogHelpers.textInputDialog(
            addDivider,
            "Add Playlist Divider",
            "Enter a name for the new divider.",
            the_session.getNextName("Divider {}"),
            ["Cancel", "Add"])
    }

}