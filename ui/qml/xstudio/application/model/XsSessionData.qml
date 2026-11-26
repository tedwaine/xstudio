import QtQuick
import QuickFuture 1.0

import xstudio.qml.session 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0
import xstudio.qml.global_store_model 1.0
import xstudio.qml.bookmarks 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.embedded_python 1.0

Item {

    /* This model gives us access to the playlists */
    XsSessionModelNode {
        id: mediaSelectionThingy  
        childSearchPropertyKey: "typeRole"
        childSearchPropertyValue: "PlayheadSelection"
        rootActor: inspectedContainerActor
        property var selectedMediaUuids: values.selectedMediaRole
        function setMediaSelection(selectedMedia) {
            values.selectedMediaRole = selectedMedia
        }
    }
    property alias mediaSelectionThingy: mediaSelectionThingy
    property var mediaSelection: mediaSelectionThingy.values.selectedMediaRole


    /* This model gives us access to the playlists */
    XsSessionModelNode {
        id: playlistsModelData    
        childSearchPropertyKey: "nameRole"
        childSearchPropertyValue: "PlaylistsRoot"
    }
    property alias playlistsModelData: playlistsModelData
    property var inspectedContainerActor: the_session.values.inspectedContainerRole

    XsSessionModelNode {
        id: __inspectedContainer  
        rootActor: inspectedContainerActor
    }
    property alias inspectedContainerProperties: __inspectedContainer.values

    XsSessionModelNode {
        id: __mediaModelData    
        rootActor: inspectedContainerActor
        childSearchPropertyKey: "nameRole"
        childSearchPropertyValue: "Media List"
    }

    XsMediaListFilterModel {
        sourceModel: __mediaModelData
        searchString: mediaListSearchString
        id: filteredModel
    }
    property alias filteredmediaModelData: filteredModel

    // Acess to the session root
    XsSessionModelNode {
        id: the_session
    }
    property alias the_session: the_session

    // Access to the playlist/subset/timeline that is currently 
    // driving the viewport
    property var viewedContainerActor: the_session.values.viewedContainerRole
    XsSessionModelNode {
        id: viewedContainer
        rootActor: viewedContainerActor
    }
    property alias viewedContainer: viewedContainer

    // Access to the playlist/subset/timeline that is currently 
    // filling the MediaList panel
    XsSessionModelNode {
        id: inspectedContainer
        rootActor: the_session.values.inspectedContainerRole
    }
    property alias inspectedContainer: inspectedContainer

    /* This selection model tells us which playlist(s), contact sheets, timelines etc
    are currently selected in the UI */
    ItemSelectionModel {
        id: playlistsSelectionModel
        model: playlistsModelData
        onCurrentIndexChanged: {
                        // set the viewed container in the backend
            var py_args = {
                    "actor": playlistsModelData.getData(currentIndex, "actorRole")
                }
            Future.promise(
                helpers.pythonAsyncCallback("UtilityPythonPlugin", "set_inspected_container", py_args)
            ).then(function(result) {
            },
	        function() {
                console.log("ERROR", __rr)
	        })

        }
        onSelectionChanged: {
            //sessionData.setSessionSelection(sessionSelectionModel.selectedIndexes)
        }
    }
    property alias playlistsSelectionModel: playlistsSelectionModel
    
}