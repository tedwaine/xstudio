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

    property string mediaListSearchString

    /* This model gives us access to the playlists */
    XsSessionModelNode {
        id: mediaSelectionThingy  
        childSearchPropertyKey: "typeRole"
        childSearchPropertyValue: "PlayheadSelection"
        rootActor: inspectedContainerActor
        property var selectedMediaUuids: values.selectedMediaRole
        function setMediaSelection(selectedMedia) {
            console.log("Setting media selection to: ", selectedMedia);
            values.selectedMediaRole = selectedMedia
        }
    }
    property alias mediaSelectionThingy: mediaSelectionThingy
    property var mediaSelection: mediaSelectionThingy.values.selectedMediaRole

    // Set the current selection to be just the single hero media currently being viewed
    function soloSelection() {
        for (var i = 0; i < filteredmediaModelData.rowCount(); i++) {
            if (filteredmediaModelData.get(i, "uuidRole") == currentPlayhead.mediaUuid) {
                mediaSelectionThingy.setMediaSelection([filteredmediaModelData.get(i, "uuidRole")])
                return;
            }
        }
    }

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
    property alias viewedContainerProperties: viewedContainer.values

    // Access to the playlist/subset/timeline that is currently 
    // filling the MediaList panel
    XsSessionModelNode {
        id: inspectedContainer
        rootActor: the_session.values.inspectedContainerRole
    }
    property alias inspectedContainer: inspectedContainer

    XsPlayhead {
        id: currentPlayhead
        //uuid: viewedContainer.rootData.activePlayheadActorRole
        playheadActor: viewedContainer.values.activePlayheadActorRole
    }
    property alias currentPlayhead: currentPlayhead

    // Access to the playlist/subset/timeline that is currently 
    // filling the MediaList panel
    XsSessionModelNode {
        id: __currentOnScreenMediaData
        rootActor: currentPlayhead.currentMediaActor
    }
    property alias currentOnScreenMediaData: __currentOnScreenMediaData

    XsSessionModelNode {
        id: __currentOnScreenMediaSourceData
        rootActor: currentPlayhead.currentMediaSourceActor
    }
    property alias currentOnScreenMediaSourceData: __currentOnScreenMediaSourceData

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