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
        function() {
            console.log("ERROR", __rr)
        })

    }

}