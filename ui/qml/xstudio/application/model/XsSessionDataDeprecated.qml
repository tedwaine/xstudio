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

    id: collecion
    property var sessionActorAddr

    /*XsSessionModelNew {
        id: sessionDataNode
    }*/

    XsSessionModel {

        id: sessionData
        sessionActorAddr: collecion.sessionActorAddr

        // This index points to the list of playlists within the session
        property var playlistsRootIdx: helpers.makePersistent(index(0, 0, index(-1, -1)))

        // Not sure if we need to update the playlists root idx on ALL of these
        // signals. One thing we can't do is automatically maintain a 'static'
        // index without triggering off these signals or using a DelegateModel
        onModelReset: {
            playlistsRootIdx = helpers.makePersistent(index(0, 0, index(-1, -1)))
        }
        onCountChanged: {
            playlistsRootIdx = helpers.makePersistent(index(0, 0, index(-1, -1)))
        }

        function checkCurrentMediaContainer(retry) {

            if(currentMediaContainerIndex.valid) {
                // wait for valid index..

            } else if (retry) {
                updateCurrentMediaContainerIndexFromBackend()
                updateViewportCurrentMediaContainerIndexFromBackend()
                // backend actor may have told us the current playlist has
                // changed, but the playlist hasn't been added to the UI model
                // yet. Check back in 200 ms.
                callbackTimer.setTimeout(function() { return function() {
                    checkCurrentMediaContainer(true)
                }}(), 200);
            } else {
                // clear the selection
                // sessionSelectionModel.clear()
            }
        }

        onCurrentMediaContainerChanged: {

            checkCurrentMediaContainer(true);

            // get the index of the PlayheadSelection node for the inspected playlist
            let ind = session.searchRecursive("PlayheadSelection", "typeRole", currentMediaContainerIndex)

            if (ind.valid) {
                // make the 'mediaSelectionModel' track the PlayheadSelection
                playheadSelectionIndex = helpers.makePersistent(ind)

            }

            // if currentMediaContainerIndex is a Subset or Timeline, get
            // it's parent playlist
            let pl = session.getPlaylistIndex(currentMediaContainerIndex)

            // if indeed currentMediaContainerIndex is a Subset or Timeline
            // we want to ensure that the parent playist is expanded
            if(pl != currentMediaContainerIndex)
                session.set(pl, true, "expandedRole")

        }

        function createPlaylist(name, sync=true, select=true) {

            // always add at end of playlists list
            var insert_row = rowCount(index(0,0));

            var idx = insertRowsSync(
                insert_row,
                1,
                "Playlist",
                "",
                name,
                index(0,0)
            )[0]

            if(select)
                sessionSelectionModel.setCurrentIndex(
                    idx,
                    ItemSelectionModel.ClearAndSelect)

            return idx
        }
        
        function createPlaylistChild(name, type, rate, playlistIndex, insert_before) {
            // note row 2 of Playlist in tree is the list of subsets, contactsheets etc

            var newidx = sessionData.insertRowsSync(
                insert_before==undefined ? sessionData.rowCount(sessionData.index(2, 0, playlistIndex)) : insert_before,
                1, type, rate, name,
                sessionData.index(2,0, playlistIndex)
            )

            // hideous!!
            callbackTimer.setTimeout(function(index) { return function() {
                sessionData.fetchMore(index)
                sessionData.fetchMore(index.parent)
                sessionSelectionModel.setCurrentIndex(
                    index,
                    ItemSelectionModel.ClearAndSelect)

                }}( newidx[0] ), 200);
            return newidx[0];
        }

        function createSubItem(name, type, rate="") {
            if (currentMediaContainerIndex.valid) {

                var selectedType = sessionData.get(currentMediaContainerIndex, "typeRole")
                var result
                if (selectedType== "Playlist") {
                    // A playlist is selected. Add as a child.
                    result = createPlaylistChild(name, type, rate, currentMediaContainerIndex)
                } else if (["Subset", "Timeline", "ContactSheet"].includes(selectedType)) {
                    // Subset or similar is selected. Insert after
                    result = createPlaylistChild(
                        name,
                        type,
                        rate,
                        currentMediaContainerIndex.parent.parent,
                        currentMediaContainerIndex.row+1
                        )

                } else {
                    // Not sure what's selected. try to find playlist?
                    var p = currentMediaContainerIndex
                    while (p.valid) {
                        if (p.model.get(p, "typeRole") == "Playlist") {
                            result = createPlaylistChild(name, type, rate, p)
                            break
                        }
                        p = p.parent
                    }
                }

                if (result && result.valid) {
                    theSessionData.set(result.parent.parent, true, "expandedRole")
                }
                return result

            }
            // can't find a selected playlist to add to. Need a new playlist
            var p = createPlaylist(the_session.getNextName("Playlist {}"))
            theSessionData.set(p, true, "expandedRole")
            // need a delay to allow the playlist node in the model to be
            // filled out by the backend
            callbackTimer.setTimeout(function(name, type, p) { return function() {
                createPlaylistChild(name, type, rate, p)
                }}( name, type, p ), 200);
        }

        function createDivider(name) {
            if (currentMediaContainerIndex.valid) {
                sessionData.insertRowsSync(
                    currentMediaContainerIndex.row+1,
                    1, "ContainerDivider", "", name,
                    currentMediaContainerIndex.parent
                    )
            } else {
                sessionData.insertRowsSync(
                    0,
                    1, "ContainerDivider", "", name,
                    playlistsRootIdx
                    )
            }
        }

    }

    property alias currentMediaContainerIndex: sessionData.currentMediaContainerIndex
    property alias viewportCurrentMediaContainerIndex: sessionData.viewportCurrentMediaContainerIndex

    //property alias sessionNew: sessionDataNode
    property alias session: sessionData

    property var currentTimelineSelection: null

    /* playheadSelectionIndex is the index into the model that points to the 'active'
    playheadSelectionActor - Each playlist, subset, timeline has its own
    playheadSelectionActor and this is the object that selectes media from the
    playlist to be shown in the viewport (and compared with A/B, String compare
    modes etc.) */
    property var playheadSelectionIndex: session.index(-1, -1)
    XsModelProperty {
        id: __selectedMedia
        index: playheadSelectionIndex
        role: "selectionRole"
    }
    property var playlistSelectedMediaActors: __selectedMedia.value
    onPlaylistSelectedMediaActorsChanged: {
        if (playlistSelectedMediaActors != undefined) {
            mediaSelectionModel.updateSelection(playlistSelectedMediaActors)
        }
    }

    /* This XsModuleData talks to a backend data model that contains all the
    attribute data of the Playhead object and exposes it as data in QML as
    a QAbstractItemModel. Every playhead instance in the app publishes its own
    data model which is identified by the uuid of the playhead - by changing the
    'modelDataName' to the Uuid of the current playhead we get access to the
    data of the current active playhead.

    If this seems confusing it's because it is! We have two different ways of
    exposing the data of backend objects - the main Session model and then more
    flexible 'XsModuleData' that can be set-up (in the backend) to include some
    or all of the data from one or several backend objects.

    At some point we may rationalise this and build into the singe Session model*/
    XsPlayhead {
        id: current_playhead
        //uuid: viewedContainer.rootData.activePlayheadActorRole
        playheadActor: viewedContainer.values.activePlayheadActorRole
        onMediaUuidChanged: {
            current_onscreen_media_data.index = session.searchRecursive(mediaUuid, "actorUuidRole", sessionData.viewportCurrentMediaContainerIndex)
            if (!current_onscreen_media_data.index.valid) {
                callbackTimer.setTimeout(function() { return function() {
                    current_onscreen_media_data.index = session.searchRecursive(mediaUuid, "actorUuidRole", sessionData.viewportCurrentMediaContainerIndex)
                    }}(), 200);
            }
        }
    }
    property alias current_playhead: current_playhead

    XsModelPropertyMap {
        id: current_onscreen_media_data
        property var imageActorUuid: values.imageActorUuidRole ? values.imageActorUuidRole : undefined
        onImageActorUuidChanged: {
            current_onscreen_media_source_data.index = session.searchRecursive(imageActorUuid, "actorUuidRole", index)
            if (!current_onscreen_media_source_data.index.valid) {
                // oh dear ... model data behind sessionData hasn't been filled
                // in for the media sources of the on-screen media item.
                // Let's fetch them
                session.fetchMore(index)
                callbackTimer.setTimeout(function(idx) { return function() {
                    current_onscreen_media_source_data.index = session.searchRecursive(imageActorUuid, "actorUuidRole", idx)
                    }}(index), 100);
            }
        }
    }
    property alias currentOnScreenMediaData: current_onscreen_media_data

    XsModelPropertyMap {
        id: current_onscreen_media_source_data
    }
    property alias currentOnScreenMediaSourceData: current_onscreen_media_source_data


    // This ItemSelectionModel manages media selection within the current
    // active playlist/subset/timeline etc.
    ItemSelectionModel {

        id: mediaSelectionModel
        model: session

        property bool updateBackend: true
        property var lastContainerWithUserSelection

        onSelectionChanged: {

            // auto populate media sources.
            selectedIndexes.forEach(function (item, index) {
                 if(session.canFetchMore(item))
                    session.fetchMore(item)
            })

            if (updateBackend) {
                lastContainerWithUserSelection = currentMediaContainerIndex
                session.updateSelection(playheadSelectionIndex, selectedIndexes)
            }
        }

        property bool multiSelected: selection.length > 1

        // this method is called by some functions that load media into a playlist.
        // The idea is that when the loading is complete, we want to select the
        // first item in the playlist for display. However .... what happens if
        // the user has already selected something before the playlist has
        // finished building? In that case, we don't want to override their
        // selection.
        // As such we make a note of the mediacontainer index when the user does
        // a selection... if the index hasn't changed since then we don't
        // so anything here.
        function selectFirstNewMedia(index, quuids) {
            if (lastContainerWithUserSelection != currentMediaContainerIndex) {
                selectNewMedia(index, [quuids[0]])
            }
        }

        function selectNewMedia(index, quuids, playhead_index=-1, mode=ItemSelectionModel.ClearAndSelect) {

            // see note above, we don't want to change the selection automatically if
            // the user has made their own selection
            if (lastContainerWithUserSelection != currentMediaContainerIndex) return;

            let type = index.model.get(index,"typeRole")
            if(quuids.length && ["Playlist", "Subset", "Timeline", "ContactSheet"].includes(type)) {
                // selects the playlist (or subset or timeline) corresponding to 'index'
                // mediaSelectionModel.select(
                //     helpers.createItemSelection([index]),
                //     ItemSelectionModel.ClearAndSelect | ItemSelectionModel.setCurrentIndex)

                callbackTimer.setTimeout(function(plindex, new_items) {
                    return function() {
                        let indexes = []

                        for(let i = 0;i<new_items.length;i++) {
                            let mi = plindex.model.searchRecursive(
                                new_items[i],
                                "actorUuidRole", plindex.model.index(0,0,plindex)
                            )
                            if(mi.valid)
                                indexes.push(mi)
                        }

                        if(indexes.length) {
                            mediaSelectionModel.select(
                                helpers.createItemSelection(indexes),
                                mode | ItemSelectionModel.setCurrentIndex
                            )
                        }

                        if(playhead_index != -1)
                            current_playhead.keySubplayheadIndex = playhead_index
                    }
                } ( index, quuids ), 250);
            }
        }

        // This function is called when the selectionRole data of the current
        // PlaylistSelection actor changes. The selectionRole data is a list
        // of uuids of media items that are selected. It should match the
        // selection of THIS (the mediaSelectionModel) but if the selection has
        // been changed in the backend only then we need to update ourselves.
        function updateSelection(uuids) {

            // first get uuids of our frontend selection
            var selected_uuids = []
            for (var i in selectedIndexes) {
                selected_uuids.push(
                    sessionData.get(
                        selectedIndexes[i],
                        "actorUuidRole")
                        )
            }

            // check if it already matches the incoming backend selection
            var needSync = false
            if (selected_uuids.length == uuids.length) {
                for (var i in selected_uuids) {
                    if (selected_uuids[i] != uuids[i]) {
                        needSync = true
                        break
                    }
                }
            } else {
                needSync = true
            }

            if (!needSync) return

            // make sure we don't trigger backend updates as we update
            // the mediaSelectionModel
            updateBackend = false
            let new_selection = []
            for (var i in uuids) {
                let idx = sessionData.searchRecursive(
                    uuids[i],
                    "actorUuidRole", currentMediaContainerIndex,
                    0,
                    2 // depth of 2 - stops the search hitting a match in a subset if we are searching a playlist
                    )

                if (idx.valid) {
                    new_selection.push(idx)
                }
            }

            mediaSelectionModel.select(
                helpers.createItemSelection(new_selection),
                ItemSelectionModel.ClearAndSelect
            )

            updateBackend = true
        }

        function getSourceIndex(mediaIndex) {
            let m = mediaIndex.model
            let ms = m.searchRecursive(m.get(mediaIndex, "imageActorUuidRole"), "actorUuidRole", mediaIndex)
            return ms
        }


        // Set the current selection to be just the single hero media currently being viewed
        function soloSelection() {
            if (!mediaSelectionModel.selectedIndexes.length) return;
            var row = mediaListModelData.getRowWithMatchingUuid(currentPlayhead.mediaUuid)
            mediaSelectionModel.select(
                helpers.createItemSelection([mediaListModelData.rowToSourceIndex(row)]),
                ItemSelectionModel.ClearAndSelect
            )
        }
    }

    property alias mediaSelectionModel: mediaSelectionModel


    /* This model gives access to bookmark/notes data */
    XsBookmarkModel {
        id: bookmarkModel
        bookmarkActorAddr: sessionData.bookmarkActorAddr
    }
    property alias bookmarkModel: bookmarkModel


}