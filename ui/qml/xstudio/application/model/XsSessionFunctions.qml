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

}