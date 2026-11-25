// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQml.Models 2.15
import xstudio.qml.helpers 1.0
import xstudio.qml.clipboard 1.0
import QuickFuture 1.0
import xStudio 1.0

Item {
    XsPreference {
        id: sessionLinkPrefix
        path: "/core/session/session_link_prefix"
    }

    XsPreference {
        id: sessionRate
        path: "/core/session/media_rate"
    }

    XsPreference {
        id: chunkSize
        path: "/ui/qml/contactsheet_chunk_size"
    }

    Clipboard {
      id: clipboard
    }


    function addtoNewPlaylistContext() {
        dialogHelpers.textInputDialog(
            function(name, button) {

                if (button == "Cancel") return
                Future.promise(
                    helpers.pythonAsyncCallback("UtilityPythonPlugin", "move_or_copy_media_to_new_playlist", {
                        "new_playlist_name": name,
                        "copy": button == "Copy Media"})
                ).then(function(return_val) {

                },
                function(__rr) {
                    dialogHelpers.errorDialogFunc("Save Selected Playlists", __rr)
                }) 
            },
            "Add To New Playlist",
            "Enter New Playlist Name",
            the_session.getNextName("Playlist {}"),
            ["Cancel", "Move Media", "Copy Media"])
    }


    function duplicateSelectedMedia() {
        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "duplicate_selected_media")
        ).then(function(return_val) {

        },
        function(__rr) {
            dialogHelpers.errorDialogFunc("Duplicate Selection", __rr)
        }) 
    }

    function relinkMedia(loose=false) {
       dialogHelpers.showFolderDialog(
            function(path, chaserFunc) {

                Future.promise(
                    helpers.pythonAsyncCallback("UtilityPythonPlugin", "relink_selected_media", {
                        "path": path,
                        "loose": loose
                    })
                ).then(function(return_val) {

                },
                function(__rr) {
                    dialogHelpers.errorDialogFunc("Relink Media", __rr)
                }) 
            },
            file_functions.defaultSessionFolder(),
            "Relink media files")
    }


    // FIXME - need to look up active source/stream
    function setMediaRotation(rotation, button) {
        if (button == "Cancel") return
        var rot = typeof rotation === "string" ? parseFloat(rotation) : rotation
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                // two steps down to first media stream of first media source
                element.model.setRoleData(element.model.index(0,0,element.model.index(0,0,element)), rot, "rotationRole")
            }
        )        
    }

    // FIXME - need to look up active source/stream
    function copyQuickViewToClipboard() {
        let filenames = []
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                // noe step down to first media source
                filenames.push(helpers.pathFromURL(element.model.getData(element.model.index(0,0,element), "pathShakeRole")))
            }
        )        
        clipboard.text = "xstudio -l '" + filenames.join("' '") +"'"
    }

    function copyMediaNamesToClipboard() {
        let result = []
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                result.push(helpers.pathFromURL(element.model.getData(element.model.index(0,0,element), "nameRole")))
            }
        )   
        clipboard.text = result.join("\n")
    }

    // FIXME - need to look up active source/stream
    function copyFilePathsToClipboard() {

        let filenames = []
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                // noe step down to first media source
                filenames.push(helpers.pathFromURL(element.model.getData(element.model.index(0,0,element), "pathShakeRole")))
            }
        )        
        clipboard.text = filenames.join("\n")
    }

    function copyEmailLinkToClipboard() {
        let name = encodeURIComponent(inspectedContainerProperties.nameRole)
        let prefix = "&" + name + "_media="
        let filenames = getSelectedMediaUrl()
        for(let i =0;i<filenames.length;i++) {
            filenames[i] = helpers.pathFromURL(filenames[i])
        }

        clipboard.text = sessionLinkPrefix.value + "xstudio://add_media?compare="+
            encodeURIComponent(currentPlayhead.compareMode)+"&playlist=" +
            name + prefix +
            filenames.join(prefix)
    }

    function copyColumnToClipboard(column) {
        let data = []
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                // noe step down to first media source
                var fields = element.model.getData(element, "mediaDisplayInfoRole")
                data.push(fields[column])
            }
        )        
        clipboard.text = data.join("\n")
    }

    // FIXME - find active image source
    function setMediaAspect() {

        let mi = filteredmediaModelData.selectedSourceIndexes[0]
        let ms = mi.model.index(0, 0, mi)

        dialogHelpers.numberInputDialog(
            function(new_pixel_aspect, button) {
                if(button == "Set Pixel Aspect") {
                    new_pixel_aspect = parseFloat(new_pixel_aspect)
                    filteredmediaModelData.selectedSourceIndexes.forEach(
                        (element) => {
                            element.model.set(element.model.index(0, 0, element), new_pixel_aspect, "pixelAspectRole")
                        }
                    )
                }
            },
            "Set Pixel Aspect Ratio",
            "Enter new pixel aspect ratio to apply to selected media",
             mi.model.get(ms, "pixelAspectRole"),
            ["Cancel", "Set Pixel Aspect"])
    }

    function setMediaFPS() {
        let mi = filteredmediaModelData.selectedSourceIndexes[0]
        let ms = mi.model.index(0, 0, mi)

        dialogHelpers.numberInputDialog(
            function(new_rate, button) {
                if(button == "Set Rate") {
                    filteredmediaModelData.selectedSourceIndexes.forEach(
                        (element) => {
                            element.model.set(element.model.index(0, 0, element), new_rate, "rateFPSRole")
                        }
                    )
                }
            },
            "Set Media Rate",
            "Enter New Media Rate",
             mi.model.get(ms, "rateFPSRole"),
            ["Cancel", "Set Rate"])
    }

    function openInQuickView() {
        if (filteredmediaModelData.selectedSourceIndexes.length > 8) {
            dialogHelpers.errorDialogFunc(
                "Warning",
                "The quick view feature is limited to a selection of 8 or fewer media items!"
                )
            return;
        }
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                var actor = theSessionData.get(element, "actorRole")
                quick_view_launcher.launchQuickViewer([actor], "Off", -1, -1)
            }
        )
    }

    function copyFileNamesToClipboard() {
        let result = getSelectedMediaUrl("pathShakeRole")
        for(let i =0;i<result.length;i++) {
            result[i] = helpers.pathFromURL(result[i])
            result[i] = result[i].substr(result[i].lastIndexOf("/")+1)
        }
        clipboard.text = result.join("\n")
    }

    function addToNewPlaylistDialog() {
        dialogHelpers.textInputDialog(
            function(new_name, button) {
                if (button == "Add") {
                    addToNewPlaylist(new_name)
                }
            },
            "Add Playlist",
            "Enter a name for the new playlist.",
            the_session.getNextName("Playlist {}"),
            ["Cancel", "Add"])
    }

    function addToNewSubsetDialog() {
        dialogHelpers.textInputDialog(
            function(new_name, button) {
                if (button == "Add") {
                    addToNewSubset(new_name)
                }
            },
            "Add Subset",
            "Enter a name for the new subset.",
            the_session.getNextName("Subset {}"),
            ["Cancel", "Add"])
    }

    function addToNewContactSheetDialog() {
        dialogHelpers.textInputDialog(
            function(new_name, button) {
                if (button == "Add") {
                    addToNewContactSheet(new_name)
                }
            },
            "Add Contact Sheet",
            "Enter New Contact Sheet Name.",
            the_session.getNextName("Contact Sheet {}"),
            ["Cancel", "Add"])
    }

    function addToNewContactSheetsDialog() {
        dialogHelpers.contactSheetDialog(
            function(name, count, button) {
                if(button == "Add") {
                    let icount = parseInt(count);
                    if(chunkSize.value != icount)
                        chunkSize.value = icount;

                    addToNewContactSheet(name, icount);
                }
            },
            "Add Contact Sheets",
            "Enter New Contact Sheets Name",
            "Contact Sheet {}",
            chunkSize.value,
            ["Cancel", "Add"])
    }

    function addToNewSequenceDialog() {

        let rate = sessionRate.value

        if(filteredmediaModelData.selectedSourceIndexes.length) {
            let idx = filteredmediaModelData.selectedSourceIndexes[0]
            let image_source = idx.model.index(0, 0, idx)
            rate = idx.model.getData(image_source, "rateFPSStringRole")
        }

        dialogHelpers.sequenceInputDialog(
            function(new_name, fps, button) {
                if (button == "Add") {
                    addToNewSequence(new_name, fps)
                }
            },
            "Add Sequence",
            "New Sequence",
            the_session.getNextName("Sequence {}"),
            ["Cancel", "Add"],
            rate)
    }

    function selectAll() {
        var new_selection = []
        for (var i = 0; i < filteredmediaModelData.rowCount(); ++i) {            
            let idx = filteredmediaModelData.rowToSourceIndex(i)
            new_selection.push(idx.model.getData(idx, "uuidRole"))
        }
        console.log(new_selection)
        mediaSelectionThingy.setMediaSelection(new_selection)        
    }

    function invertSelection() {
        var new_selection = []
        for (var i = 0; i < filteredmediaModelData.rowCount(); ++i) {            
            let idx = filteredmediaModelData.rowToSourceIndex(i)
            let selectionIndex = idx.model.getData(idx, "selectionIndexRole") 
            if (!selectionIndex) {
                new_selection.push(idx.model.getData(idx, "uuidRole"))
            }

        }
        mediaSelectionThingy.setMediaSelection(new_selection)        
    }

    // FIXME - lookup image source properly
    function getOffline() {

        var selection = []
        for (var i = 0; i < filteredmediaModelData.rowCount(); ++i) {
            let idx = filteredmediaModelData.rowToSourceIndex(i)
            let state = idx.model.getData(idx.model.index(0, 0, idx), "mediaStatusRole")
            if(state != undefined && state != "Online")
                selection.push(idx)
        }   
        return selection
    }

    function selectAllOffline() {
        let offlineMedia = getOffline()
        var new_selection = []
        offlineMedia.forEach(
            (element) => {
                new_selection.push(element.model.getData(element, "uuidRole"))
            }
        )
        mediaSelectionThingy.setMediaSelection(new_selection)
    }

    // FIXME - looks like this was already broke
    function selectAllAdjusted() {
        var selection = []
        for (var i = 0; i < filteredmediaModelData.rowCount(); ++i) {
            let idx = filteredmediaModelData.rowToSourceIndex(i)
            let bk = idx.model.getData(idx, "bookmarkUuidsRole")
            if(bk != undefined && bk.length)
                selection.push(idx.model.getData(idx, "uuidRole"))
        }   
        mediaSelectionThingy.setMediaSelection(selection)
    }

    function selectFlagged(flag) {
        var selection = []
        for (var i = 0; i < filteredmediaModelData.rowCount(); ++i) {
            let idx = filteredmediaModelData.rowToSourceIndex(i)
            let f = idx.model.getData(idx, "flagColourRole")
            if(f == flag)
                selection.push(idx.model.getData(idx, "uuidRole"))
        }   
        mediaSelectionThingy.setMediaSelection(selection)
    }

    function setColour(indexes, idx) {
        indexes.forEach(
            function (item, index) {
                theSessionData.set(item, flagColours[idx].colour, "flagColourRole")
            }
        )
    }

    function cycleColour(indexes) {
        var colours = flagColours.map(obj => Object.values(obj)[0])
        indexes.forEach(
            function (item, index) {
                let current = theSessionData.get(item, "flagColourRole")
                let cind = colours.indexOf(current)
                if(cind == -1 || cind == colours.length-1)
                    theSessionData.set(item, colours[0], "flagColourRole")
                else
                    theSessionData.set(item, colours[cind+1], "flagColourRole")
            }
        )
    }

    function deselectAll() {
        mediaSelectionThingy.setMediaSelection([])
    }

    function mediaIndexAfterRemoved(indexes) {

        let select_row = -1;
        let to_remove = []
        let parent = indexes[0].parent;

        for(let i =0; i<indexes.length; ++i)
            to_remove.push(indexes[i].row)

        to_remove = to_remove.sort(function(a,b){return b-a})

        while(select_row == -1 && to_remove.length) {
            select_row = to_remove[0] + 1
            to_remove.shift()
        }

        if (select_row == indexes.length)
            select_row = -1

        return parent.model.index(select_row, 0, parent)
    }

    function deleteSelected(force=false) {
        if(force)
            deleteSelectedCallback("Delete", filteredmediaModelData.selectedSourceIndexes)
        else {
            dialogHelpers.multiChoiceDialog(
                function(response) {deleteSelectedCallback(response, filteredmediaModelData.selectedSourceIndexes)},
                "Delete Media",
                "Remove the selected media?",
                ["Cancel", "Delete"],
                undefined)
        }
    }

    function deleteOffline(force=false) {
        if(force)
            deleteIndexes(getOffline())
        else
            dialogHelpers.multiChoiceDialog(
                deleteOfflineCallback,
                "Delete Offline Media",
                "Remove the offline media?",
                ["Cancel", "Delete"],
                undefined)

    }

    function deleteOfflineCallback(response) {
        if (response == "Delete")
            deleteIndexes(getOffline())
    }

    function deleteIndexes(indexes) {
        let uuids = []
        indexes.forEach(
            (element) => {
                // noe step down to first media source
                uuids.push(element.model.getData(element, "uuidRole"))
            }
        )
        Future.promise(
                helpers.pythonAsyncCallback("UtilityPythonPlugin", "delete_selected_media", {"uuids": uuids})
            ).then(function(result) {
            },
	        function(__rr) {
                dialogHelpers.errorDialogFunc("Delete Media", __rr)
	        })
    }

    function deleteSelectedCallback(response, indexes) {
        if (response == "Delete")
            deleteIndexes(indexes)
    }

    function selectUp() {

        if (!filteredmediaModelData.selectedSourceIndexes.length) return;
        var first_row = 10000000
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                let row = filteredmediaModelData.sourceIndexToRow(element)
                if (row < first_row)
                    first_row = row
            }
        )
        if (first_row > 0) {
            let uuid = filteredmediaModelData.getData(first_row-1, "uuidRole")
            let new_selection = mediaSelection
            new_selection.push(uuid)
            mediaSelectionThingy.setMediaSelection(new_selection)
        }
    }

    function selectDown() {

        if (!filteredmediaModelData.selectedSourceIndexes.length) return;
        var last_row = -10000000
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                let row = filteredmediaModelData.sourceIndexToRow(element)
                if (row > last_row)
                    last_row = row
            }
        )
        if (last_row < (filteredmediaModelData.rowCount() - 1)) {
            let uuid = filteredmediaModelData.getData(last_row+1, "uuidRole")
            let new_selection = mediaSelection
            new_selection.push(uuid)
            mediaSelectionThingy.setMediaSelection(new_selection)
        }
    }

    function getSelectedMediaUrl(role="pathRole", sourceName="") {
        let result = []
        filteredmediaModelData.selectedSourceIndexes.forEach(
            (element) => {
                // noe step down to first media source
                let r = element.model.getData(element.model.index(0,0,element), role)
                if (r != undefined)
                    result.push(r)
            }
        )  
        return result;
    }

    function addToNewSequence(name, rate) {

        Future.promise(
                helpers.pythonAsyncCallback("UtilityPythonPlugin", "add_selected_media_to_new_object", {"name": name, "rate": rate, "object_type": "Timeline"})
            ).then(function(result) {
            },
	        function(__rr) {
            dialogHelpers.errorDialogFunc("Add to New Sequence", __rr)
	        })

    }

    function addToNewSubset(name) {
        Future.promise(
                helpers.pythonAsyncCallback("UtilityPythonPlugin", "add_selected_media_to_new_object", {"name": name, "object_type": "Subset"})
            ).then(function(result) {
            },
	        function(__rr) {
                dialogHelpers.errorDialogFunc("Add to New Subset", __rr)
	        })
    }


    function addToNewContactSheet(name, count = -1) {
        Future.promise(
                helpers.pythonAsyncCallback("UtilityPythonPlugin", "add_selected_media_to_new_object", {"name": name, "object_type": "ContactSheet", "count": count})
            ).then(function(result) {
            },
	        function(__rr) {
                dialogHelpers.errorDialogFunc("Add to New Contact Sheet", __rr)
	        })
    }

    function addToNewPlaylist(name) {
        Future.promise(
                helpers.pythonAsyncCallback("UtilityPythonPlugin", "add_selected_media_to_new_object", {"name": name, "object_type": "Playlist"})
            ).then(function(result) {
            },
	        function(__rr) {
                dialogHelpers.errorDialogFunc("Add to New Playlist", __rr)
	        })
    }

}