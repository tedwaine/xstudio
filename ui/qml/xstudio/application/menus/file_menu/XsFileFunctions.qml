
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
    XsPreference {
        id: checkUnsaved
        path: "/ui/qml/check_unsaved_session"
    }

    function newSession() {
        studio.newSession("New Session")
        studio.clearImageCache()
    }

    function modSaveWarning(incoming) {
        if (sessionPath != undefined) {
            return incoming.replace("This Session has been modified", "This Session (" + sessionPath + ") has been modified")
        }
        return incoming
    }

    function newSessionWithCheck() {

        if (theSessionData.modified) {

            dialogHelpers.multiChoiceDialog(
                newSessionWithCheckCallback,
                "New Session",
                modSaveWarning("This Session has been modified. Would you like to save it before loading a new one?"),
                ["Cancel", "Don't Save", "Save"],
                undefined)
        } else {
            newSession()
        }

    }

    function newSessionWithCheckCallback(response, chaser) {
        if (response == "Save") {
            saveSessionCheck(newSession)
        } else if (response == "Don't Save") {
            newSession()
        }
    }

    function exportSequencePath(chaserFunc=undefined) {
        dialogHelpers.showFileDialog(
            function(fileUrl, undefined, func) {
                exportSequence(fileUrl, "", func)
            },
            defaultSessionFolder(),
            "Export Sequence",
            ".otio",
            ["All Files (*.*)"].concat(theSessionData.getTimelineExportTypes()),
            false,
            false,
            chaserFunc
        )
    }

    function exportSequence(path, type, chaserFunc) {
        if (path == false) return; // save was cancelled

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "export_otio", {
                "timeline_uuid": playlistsSelectionModel.model.getData(playlistsSelectionModel.selectedIndexes[0], "uuidRole"),
                "path": path})
        ).then(function(return_val) {

            if (return_val) {
                // do export should return [True, message]
                dialogHelpers.messageDialogFunc("OTIO Export", "File " + path + " exported successfully", "Ok")

            } else if (typeof return_val === "string") {
                // report (likely) error of some sort
                dialogHelpers.errorDialogFunc("OTIO Export", "Failed to export file " + path + ". Check log output for details.", "Ok")
            }

        },
        function(__rr) {
            dialogHelpers.errorDialogFunc("OTIO Export", __rr)
        }) 
    }

    function saveSelectionAs(path, folder, chaserFunc) {

        if (path == false) return; // save was cancelled

        var spath = "" + path
        if (!(spath.endsWith(".xst") || spath.endsWith(".xsz"))) {
            path = path + ".xst"
        }
        var playlist_ids = []
        for (var i = 0; i < playlistsSelectionModel.selectedIndexes.length; i++) {
            playlist_ids.push(playlistsSelectionModel.model.getData(playlistsSelectionModel.selectedIndexes[i], "uuidRole"))
        }
        print (playlist_ids, typeof playlist_ids[0])

        Future.promise(
            helpers.pythonAsyncCallback("UtilityPythonPlugin", "save_selected_playlists", {
                "playlist_uuids": playlist_ids,
                "path": path})
        ).then(function(return_val) {

            if (return_val) {
                // do export should return [True, message]
                dialogHelpers.messageDialogFunc("Save Selected Playlists", "File " + path + " exported successfully", "Ok")

            } else {
                // report (likely) error of some sort
                dialogHelpers.errorDialogFunc("Save Selected Playlists", "Failed to export file " + path + ". Check log output for details.", "Ok")
            }

        },
        function(__rr) {
            dialogHelpers.errorDialogFunc("Save Selected Playlists", __rr)
        }) 
    }

    function saveSessionAs(path, folder, chaserFunc) {

        if (path == false) return; // save was cancelled

        var spath = "" + path
        if (!(spath.endsWith(".xst") || spath.endsWith(".xsz"))) {
            path = path + ".xst"
        }

        Future.promise(theSessionData.saveFuture(path)).then(function(result) {
            if (result == false) {
                // cancelled
                if (chaserFunc != undefined) chaserFunc()
            } else if (result != "") {
                dialogHelpers.errorDialogFunc("Save Session Failed", result)
            } else {
                newRecentPath(path)
                if (chaserFunc != undefined) chaserFunc()
            }
        })
    }

    function saveOverwriteCallback(response, chaserFunc) {
        if (response == "Save As...") {
            saveSessionNewPath(chaserFunc)
        } else if (response == "Overwrite") {
            saveSessionAs(sessionPath, undefined, chaserFunc)
        }
    }

    function saveSessionCheck(chaserFunc) {
        if (sessionPath == undefined) {
            saveSessionNewPath(chaserFunc)
        } else if (sessionMTime.getTime() != helpers.getFileMTime(sessionPath).getTime()) {
            dialogHelpers.multiChoiceDialog(
                saveOverwriteCallback,
                "Session File Modified",
                "Session file \"" + sessionPath + "\" has been modified by something else, do you want to overwrite?",
                ["Cancel", "Overwrite", "Save As..."],
                ["Cancel", "Overwrite", "Save As..."],
                chaserFunc
                )
        } else {
            saveSessionAs(sessionPath, undefined, chaserFunc)
        }
    }

    function saveSessionNewPath(chaserFunc) {

        dialogHelpers.showFileDialog(
            saveSessionAs,
            defaultSessionFolder(),
            "Save Session",
            sessionCompression ? "xsz" : "xst",
            ["xStudio (*.xst *.xsz)"],
            false,
            false,
            chaserFunc
            )

    }

    function saveSelectionNewPath(chaserFunc) {

        dialogHelpers.showFileDialog(
            saveSelectionAs,
            defaultSessionFolder(),
            "Save Session",
            sessionCompression ? "xsz" : "xst",
            ["xStudio (*.xst *.xsz)"],
            false,
            false,
            chaserFunc
            )
    }

    function doLoadSession(path) {

        if (path == false) return; // load was cancelled

        Future.promise(studio.loadSessionFuture(path)).then(function(result) {
            if (result != true) {
                dialogHelpers.errorDialogFunc("Load Session Failed", result)
            } else {
                newRecentPath(path)
            }})
    }

    function doImportSession(path) {

        if (path == false) return; // load was cancelled

        Future.promise(theSessionData.importFuture(path, null)).then(
            function(result) {
                newRecentPath(path)
            })
    }

    function doImportOTIO(path) {

        if (path == false) return; // import was cancelled in file dialog

        let index=null

        if(index == null) {
            // create new playlist..
            index = sessionFunctions.createPlaylist("Imported OTIO")
        }

        sessionFunctions.handleDragDrop("import otio", {"text/uri-list": path}, null)

    }

    function loadSession() {

        dialogHelpers.showFileDialog(
            doLoadSession,
            defaultSessionFolder(),
            "Open Session",
            "xst",
            ["xStudio (*.xst *.xsz)"],
            true,
            false)

    }

    property var file_to_load

    function loadSessionPath() {
        doLoadSession(file_to_load)
    }

    function loadSessionPathWithCheckCallback(result) {
        if (result == "Don't Save") {
            loadSessionPath()
        } else if (result == "Save") {
            saveSessionCheck(loadSessionPath)
        }
    }

    function loadSessionFile(session_file_path, file_name) {
        if (theSessionData.modified) {
            file_to_load = session_file_path
            dialogHelpers.multiChoiceDialog(
                loadSessionPathWithCheckCallback,
                "Load Snapshot Session",
                modSaveWarning("This Session has been modified. Would you like to save it before loading " + file_name + " ?"),
                ["Cancel", "Don't Save", "Save"],
                undefined)
        } else {
            doLoadSession(session_file_path)
        }
    }

    function loadSessionWithCheck() {

        if (theSessionData.modified) {
            dialogHelpers.multiChoiceDialog(
                loadSessionWithCheckCallback,
                "Load Session",
                modSaveWarning("This Session has been modified. Would you like to save it before loading a new one?"),
                ["Cancel", "Don't Save", "Save"],
                undefined)
        } else {
            loadSession()
        }

    }

    function loadSessionWithCheckCallback(result) {
        if (result == "Don't Save") {
            loadSession()
        } else if (result == "Save") {
            saveSessionCheck(loadSession)
        }
    }

    function loadMedia(media_urls) {

        dialogHelpers.showFileDialog(
            loadMediaCallback,
            defaultMediaFolder,
            "Select Media Files",
            undefined,
            ["Media files ("+helpers.validMediaExtensions()+")", "All files (*)" ],
            true,
            true)

    }

    function loadMediaCallback(media_urls, folder) {

        if (media_urls === false) return
        sessionFunctions.loadMedia(media_urls)
        defaultMediaFolder = folder
    }

    function addSessionFromClipboard() {
        if (clipboard.text) {
            doImportSession(helpers.QUrlFromPosixPath(clipboard.text))
        }
    }

    function addMediaFromClipboard() {
        if (clipboard.data) {

            // FIXME
            let targetUuid = playlistSelectionModel.currentIndex.isValid ? playlistsModelData.getData(playlistSelectionModel.currentIndex, "uuidRole") : null
            sessionFunctions.handleDragDrop("add media", {"text/clipboard": clipboard.data}, targetUuid)

        }
    }

    function doQuit() {
        Qt.quit()
    }

    function quitWithCheckCallback(response) {
        if (response == "Quit Without Saving") {
            doQuit()
        } else if (response == "Save and Quit") {
            saveSessionCheck(doQuit)
        }
    }

    function quitWithCheck() {

        if (theSessionData.modified && checkUnsaved.value) {
            dialogHelpers.multiChoiceDialog(
                quitWithCheckCallback,
                "Quit xSTUDIO",
                modSaveWarning("This Session has been modified. Would you like to save it before closing xSTUDIO?"),
                ["Cancel", "Quit Without Saving", "Save and Quit"],
                undefined)
        } else {
            doQuit()
        }

    }

    function importSession() {
        dialogHelpers.showFileDialog(
            doImportSession,
            defaultSessionFolder(),
            "Import Session",
            "xst",
            ["xStudio (*.xst *.xsz)"],
            true,
            false)
    }

    function importOTIO() {
        dialogHelpers.showFileDialog(
            doImportOTIO,
            defaultSessionFolder(),
            "Import OTIO",
            "",
            ["All files (*)"],
            true,
            false)
    }

    function exportNotedToCSV() {
        dialogHelpers.showFileDialog(
            function(path) {
                if (path)
                    Future.promise(
                        // FIXME
                        theSessionData.getBookmarksFuture(sessionSelectionModel.selectedIndexes)
                    ).then(function(result) {
                        Future.promise(
                            bookmarkModel.exportCSVFuture(path, true, true, result)
                        ).then(function(result) {
                            dialogHelpers.errorDialogFunc("Export Notes to CSV", result)
                        })
                    })
            },
            defaultSessionFolder(),
            "Export Notes to CSV",
            "csv",
            ["CSV (*.csv)"],
            false,
            false)

    }

    function defaultSessionFolder() {
        // pick most recent path
        if(recentFiles.length && typeof recentFiles[0] == "string") {
            let p = recentFiles[0]
            return p.substr(0, p.lastIndexOf("/"))
        }
        return ""
    }

    function newRecentPath(path) {
        let old = recentFiles
        if(old == undefined || !old.length){
            old = Array()
        }
        if(old.length) {
            // remove duplicate
            old = old.filter(function(value, index, arr){
                return value != path;
            });
        }
        // add to top
        old.unshift(path)

        //prune old entries
        if(old.length > 10)
            old.pop(old.length - 10)

        recentFiles = old

    }

    function getFileNameUsingLastIndexOf(path) {
        // Find the last index of either '\' or '/'
        const lastBackslashIndex = path.lastIndexOf('\\');
        const lastSlashIndex = path.lastIndexOf('/');

        // Choose the maximum index
        const lastIndex = Math.max(lastBackslashIndex, lastSlashIndex);

        // Extract the file name using substring
        const fileName = path.substring(lastIndex + 1);

        return fileName;
    }

    XsPreference {
        id: sessionLinkPrefix
        path: "/core/session/session_link_prefix"
    }

    function doCopySessionLink() {
        clipboard.text = sessionLinkPrefix.value + "xstudio://open_session?uri=" + helpers.encodedFromQUrl(sessionPath)
    }

    function copySessionLink() {

        if(theSessionData.modified) {
            saveSessionCheck(doCopySessionLink)
        } else {
            doCopySessionLink()
        }

    }

    // Make a connection to the 'recent_history' preference (i.e. recent files)
    XsModelProperty {
        id: recent_files
        role: "valueRole"
        index: globalStoreModel.searchRecursive("/ui/qml/recent_history", "pathRole")
    }
    property alias recentFiles: recent_files.value

    XsModelProperty {
        id: default_media_folder
        role: "valueRole"
        index: globalStoreModel.searchRecursive("/ui/qml/default_media_folder", "pathRole")
    }
    property alias defaultMediaFolder: default_media_folder.value

    XsModelProperty {
        id: session_compression
        role: "valueRole"
        index: globalStoreModel.searchRecursive("/core/session/compression", "pathRole")
    }
    property alias sessionCompression: session_compression.value

    property var sessionMTime: sessionProperties.values.mtimeRole
    property var sessionPathNative: sessionPath ? helpers.pathFromURL(sessionPath) : ""

    Clipboard {
        id: clipboard
    }

}

