import QtQuick
import QtQml.Models 2.15

import xStudio 1.0
import xstudio.qml.clipboard 1.0

Item {
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
            theSessionData.createPlaylist(new_name)
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
            theSessionData.getNextName("Playlist {}"),
            ["Cancel", "Add"])
    }

    function addSubsetChoice() {
        dialogHelpers.textInputDialog(
            addSubset,
            "Add Subset",
            "Enter a name for the new subset.",
            theSessionData.getNextName("Subset {}"),
            ["Cancel", "Add"])
    }

    function addSequenceChoice() {
        dialogHelpers.sequenceInputDialog(
            addTimeline,
            "Add Sequence",
            "New Sequence",
            theSessionData.getNextName("Sequence {}"),
            ["Cancel", "Add"],
            sessionRate.value)
    }

    function addContactSheetChoice() {
        dialogHelpers.textInputDialog(
            addContactSheet,
            "Add Contact Sheet",
            "Enter a name for the new contact sheet.",
            theSessionData.getNextName("Contact Sheet {}"),
            ["Cancel", "Add"])
    }

    function addDividerChoice() {
        dialogHelpers.textInputDialog(
            addDivider,
            "Add Playlist Divider",
            "Enter a name for the new divider.",
            theSessionData.getNextName("Divider {}"),
            ["Cancel", "Add"])
    }
}