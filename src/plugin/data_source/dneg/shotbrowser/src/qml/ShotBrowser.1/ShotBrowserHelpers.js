function getProjectIdFromList(indexlist) {
	let result = 0
	if(indexlist.length) {
		let m = indexlist[0].model
		result = m.get(indexlist[0], "projectIdRole")
	}
	return result
}

function getIds(indexlist) {
	let result = []
	if(indexlist.length) {
		let m = indexlist[0].model

		for(let i=0; i<indexlist.length; i++) {
			result.push(m.get(indexlist[i], "idRole"))
		}
	}

	return result
}

function revealMediaInShotgrid(indexes=[]) {
	if(indexes.length) {
		for(let i = 0; i< indexes.length; i++) {
			let mindex = indexes[i]
	        Future.promise(
	            mindex.model.getJSONFuture(mindex, "/metadata/shotgun/version/id")
	        ).then(function(json_string) {
	            json_string = json_string.replace(/^"|"$/g, '')
	            helpers.openURL("http://shotgun/detail/Version/"+json_string)
	        })
		}
	}
}

function revealInShotgrid(indexes=[]) {
	if(indexes.length) {
		indexes = mapIndexesToResultModel(indexes)

		let m = indexes[0].model
		for(let i = 0; i< indexes.length; i++) {
			let t = m.get(indexes[i], "typeRole")

			if(["Version", "Note", "Playlist"].includes(t)) {
				let id = m.get(indexes[i], "idRole")
	            helpers.openURL("http://shotgun/detail/"+t+"/"+id)
			}
		}
	}
}

function getAllIndexes(model) {
	// need to map from display model to selection model..
	// selection model is always the result model... (I hope)
	let indexes = []

	for(let r =0; r < model.count; r++) {
		indexes.push(model.modelIndex(r))
	}
	return indexes
}

function revealMediaInIvy(indexes=[]) {
	if(indexes.length) {
		for(let i = 0; i< indexes.length; i++) {
			let mindex = indexes[i]
	        Future.promise(
	            mindex.model.getJSONFuture(mindex, "/metadata/shotgun/version/attributes/sg_ivy_dnuuid")
	        ).then(function(json_string) {
	            json_string = json_string.replace(/^"|"$/g, '')
	            helpers.startDetachedProcess("dnenv-do", [helpers.getEnv("SHOW"), helpers.getEnv("SHOT"), "--", "ivybrowser", json_string])
	        })
		}
	}
}

function revealInIvy(indexes=[]) {
	indexes = mapIndexesToResultModel(indexes)
	if(indexes.length) {
		let m = indexes[0].model

		for(let i = 0; i< indexes.length; i++) {
			let t = m.get(indexes[i], "typeRole")

			if(["Version"].includes(t)) {
				let id = m.get(indexes[i], "stalkUuidRole")
	            helpers.startDetachedProcess("dnenv-do", [helpers.getEnv("SHOW"), helpers.getEnv("SHOT"), "--", "ivybrowser", id])
			}
		}
	}
}

function getJSON(indexes=[]) {
	let jsn = []
	if(indexes.length) {
		indexes = mapIndexesToResultModel(indexes)

		let m = indexes[0].model
		for(let i = 0; i< indexes.length; i++) {
			jsn.push(m.get(indexes[i], "jsonRole"))
		}
	}
	return jsn
}

function getNote(indexes=[]) {
	let txt = ""
	if(indexes.length) {
		indexes = mapIndexesToResultModel(indexes)
		let m = indexes[0].model
		for(let i = 0; i< indexes.length; i++) {
			txt += m.get(indexes[i], "subjectRole") + "\n"
			txt += m.get(indexes[i], "contentRole") + "\n\n"
		}
	}
	return txt
}


function downloadMissingMovies(indexes=[]) {
	if(indexes.length) {
		for(let i = 0; i< indexes.length; i++) {
			let m = indexes[i].model
            // let cindex = model.index(0, 0, indexes[i])
            // let ccount = model.rowCount(cindex)
            // for(let i =0; i< ccount; ++i) {
            //     // is online.
            //     let mindex = model.index(i,0,cindex)

            if(m.get(indexes[i], "mediaStatusRole") != "Online")
                downloadMovies([indexes[i]])
            // }
        }
    }
}

function downloadMovies(indexes=[]) {
	if(indexes.length) {
		let m = indexes[0].model
		for(let i = 0; i< indexes.length; i++) {
			// console.log(m.get(indexes[i], "actorUuidRole"))

		    Future.promise(ShotBrowserEngine.addDownloadToMediaFuture(m.get(indexes[i], "actorUuidRole"))).then(
		        function(result) {
		            let jsn = JSON.parse(result)
		            if(jsn.actor_uuid)
		                m.set(indexes[i], jsn.actor_uuid, "imageActorUuidRole")
		        },
		        function() {
		        }
		    )
		}
	}
}

function mapIndexToResultModel(index) {
	let result = index

	if(index.model instanceof QTreeModelToTableModel) {
		result = mapIndexToResultModel(index.model.mapToModel(index))
	} else if(index.model instanceof ShotBrowserResultFilterModel) {
		result = mapIndexToResultModel(index.model.mapToSource(index))
	}

	return result
}


function mapIndexesToResultModel(indexes) {
	let result = indexes

	if(indexes.length) {
		let m = indexes[0].model
		if(m instanceof QTreeModelToTableModel) {
			// map to source model
			let tmp_indexes = []
			for(let i = 0; i<indexes.length; i++) {
				tmp_indexes.push(m.mapToModel(indexes[i]))
			}
			result = mapIndexesToResultModel(tmp_indexes)
		} else if(m instanceof ShotBrowserResultFilterModel) {
			let tmp_indexes = []
			for(let i = 0; i<indexes.length; i++) {
				tmp_indexes.push(m.mapToSource(indexes[i]))
			}
			result = mapIndexesToResultModel(tmp_indexes)
		}
	}

	return result
}

function nextItem(index) {
	return index.model.index(index.row + 1, 0, index.parent)
}

function compareMediaCallback(playlist_uuid, uuids) {
	// find selected media.

    if(uuids.length) {
        let plindex =  theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
	 	let tmp = []

	 	for(let i=0;i<mediaSelectionModel.selectedIndexes.length;i++)
		 	tmp.push(mediaSelectionModel.model.get(mediaSelectionModel.selectedIndexes[i], "actorUuidRole"))

		let first_new_index = mediaSelectionModel.selectedIndexes.length

	 	for(let i=0;i<uuids.length;i++)
		 	tmp.push(helpers.QVariantFromUuidString(uuids[i]))

    	mediaSelectionModel.selectNewMedia(plindex, tmp, first_new_index)

    	// find index of first new item.
		// let first_new = plindex.model.searchRecursive(helpers.QVariantFromUuidString(uuids[0]), "actorUuidRole")
		// mediaSelectionModel.select(first_new, ItemSelectionModel.setCurrentIndex)

		// the 'playheadKeySubplayheadIndex' is the index in the selection for
		// the current playhead that is put on screen (except for String compare mode)
		// Playhead has N subPlayheads

		// playheadKeySubplayheadIndex = first_new_index

    }
}

function conformToNewSequenceCallback(playlist_uuid, uuids) {
   if(uuids.length) {
        let plindex = theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
		console.log(plindex)
		let indexes = []
		for(let i=0;i<uuids.length;i++) {
			indexes.push(theSessionData.searchRecursive(uuids[i],"actorUuidRole", plindex))
		}
		appWindow.conformTool.conformToNewSequence(indexes, plindex)
	}
}


// add results and compare them
// add next to media selection ?
function compareSelectedResults(indexes=[]) {
	// insert after first media selection..
	if(viewedMediaSetProperties.values.typeRole == "Timeline") {
		// add media to playlist
		// add media on to new track based off sparse track ?
		// but only if metadata matches ..
		addToSequence(indexes)

	} else {
		if(mediaSelectionModel.selectedIndexes.length && indexes.length) {
	        let media_uuid = null
	        let next = nextItem(mediaSelectionModel.selectedIndexes[mediaSelectionModel.selectedIndexes.length-1])
	        if(next.valid) {
				media_uuid = mediaSelectionModel.model.get(next, "actorUuidRole")
	        }
			addToCurrentPlaylist(indexes, media_uuid, true, compareMediaCallback)
		}
	}
}



function replaceMediaCallback(playlist_uuid, uuids) {
	// find selected media.
    if(uuids.length && mediaSelectionModel.selectedIndexes.length) {
        let plindex =  theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
    	mediaSelectionModel.selectFirstNewMedia(plindex, uuids)

		// delete media..
    	let mi = mediaSelectionModel.selectedIndexes[0]
    	mediaSelectionModel.model.removeRows(mi.row, 1, mi.parent)
    }
}

function replaceSelectedResults(indexes=[]) {
	if(viewedMediaSetProperties.values.typeRole == "Timeline") {
		// add media to playlist
		// replace media into clip current in view port.
		// but only if metadata matches ..
		replaceToSequence(indexes)
	} else {
		if(mediaSelectionModel.selectedIndexes.length && indexes.length) {
		    let media_uuid = null
		    let next = nextItem(mediaSelectionModel.selectedIndexes[0])
		    if(next.valid) {
				media_uuid = mediaSelectionModel.model.get(next, "actorUuidRole")
		    }
			addToCurrentPlaylist([indexes[0]], media_uuid, true, replaceMediaCallback)
		}
	}
}

function selectFirstMediaCallback(playlist_uuid, uuids) {
    if(uuids.length) {
        let plindex =  theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
		sessionSelectionModel.setCurrentIndex(
			plindex,
			ItemSelectionModel.ClearAndSelect)
    	mediaSelectionModel.selectFirstNewMedia(plindex, uuids)
    }
}

function _conformMediaCallback(playlist_uuid, uuids, conformTrackIndex) {
	let sindex = theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
	if(uuids.length && sindex.model.get(sindex, "typeRole") == "Timeline") {
		// get indexes from media uuids.
		let tmp = []
		// console.log(uuids,sindex, sindex.model.rowCount(sindex), sindex.model.get(sindex.model.index(0,0,sindex), "typeRole"))
        for(let i=0;i<uuids.length;i++) {
        	let mi = sindex.model.search(uuids[i], "actorUuidRole", sindex.model.index(0,0,sindex))
        	if(mi.valid)
	            tmp.push(mi)
	        else {
	        	// retry after delay and UI hasn't updated yet...
	        	delayCallback(1000, function() {_conformMediaCallback(playlist_uuid, uuids, conformTrackIndex)})
	        	return
	        }
        }

        if(tmp.length)
			appWindow.conformTool.conformToSequence(tmp, sindex, "Added Media", conformTrackIndex)
    	else
	        console.log("appWindow.conformTool.conformToSequence", tmp, sindex, "Added Media", conformTrackIndex)
	}
}


function conformMediaCallback(playlist_uuid, uuids) {
	let sindex = theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
    let clipIndex = theSessionData.getTimelineClipIndex(sindex, currentPlayhead.logicalFrame)
    _conformMediaCallback(playlist_uuid, uuids, theSessionData.getTimelineTrackIndex(clipIndex))
}

function conformMediaToConformTrackCallback(playlist_uuid, uuids) {
	let sindex = theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
    let clipIndex = theSessionData.getTimelineClipIndex(sindex, currentPlayhead.logicalFrame)
    _conformMediaCallback(playlist_uuid, uuids, theSessionData.index(-1,-1))
}

function replaceConformMediaCallback(playlist_uuid, uuids) {
	let sindex = theSessionData.searchRecursive(playlist_uuid,"actorUuidRole")
	if(uuids.length && sindex.model.get(sindex, "typeRole") == "Timeline") {
		// get indexes from media uuids.
		let tmp = []
		// console.log(uuids,sindex, sindex.model.rowCount(sindex), sindex.model.get(sindex.model.index(0,0,sindex), "typeRole"))
        for(let i=0;i<uuids.length;i++) {
        	let mi = sindex.model.search(uuids[i], "actorUuidRole", sindex.model.index(0,0,sindex))
        	if(mi.valid)
	            tmp.push(mi)
	        else {
	        	// retry after delay and UI hasn't updated yet...
	        	delayCallback(1000, function() {replaceConformMediaCallback(playlist_uuid, uuids)})
	        	return
	        }
	    }

        // we use the current active clip to select the conform track.
        let clipIndex = theSessionData.getTimelineClipIndex(sindex, currentPlayhead.logicalFrame)
        // assumes clips are not nested..
		appWindow.conformTool.replaceToSequence(tmp, sindex, theSessionData.getTimelineTrackIndex(clipIndex))
	}
}

function _Timer() {
     return Qt.createQmlObject("import QtQuick 2.0; Timer {}", root);
}

function delayCallback(delayTime, cb) {
     let timer = new _Timer();
     timer.interval = delayTime;
     timer.repeat = false;
     timer.triggered.connect(cb);
     timer.start();
}

function selectTimelineCallback(playlist_index, uuids, wait=4) {
	playlist_index = theSessionData.getPlaylistIndex(playlist_index)
    if(uuids.length) {
    	let tindex = theSessionData.searchRecursive(helpers.QVariantFromUuidString(uuids[0]), "actorUuidRole", playlist_index)
    	if(tindex.valid) {
			sessionSelectionModel.setCurrentIndex(
				tindex,
				ItemSelectionModel.ClearAndSelect)
			// prepare timeline...
			appWindow.conformTool.conformPrepareSequence(tindex, false)
		} else if(wait) {
			delayCallback(1000, function() {
		     	selectTimelineCallback(playlist_index, uuids, wait-1)
			});
		} else {
			console.log("Failed to get timeline index", uuids, playlist_index)
		}
    }
}

function addToSequence(indexes=[], viewed=true) {
	let current_pl = viewed ? viewedMediaSetProperties.values.actorUuidRole : inspectedMediaSetProperties.values.actorUuidRole
	if(viewed)
		addToPlaylist(indexes, current_pl, null, "Untitled Playlist", conformMediaCallback)
	else
		addToPlaylist(indexes, current_pl, null, "Untitled Playlist", conformMediaToConformTrackCallback)
}

function replaceToSequence(indexes=[], callback=replaceConformMediaCallback) {
	let current_pl = viewedMediaSetProperties.values.actorUuidRole

	addToPlaylist(indexes, current_pl, null, "Untitled Playlist", callback)
}

function addToCurrentPlaylist(indexes=[], media_uuid=null, viewed=true, callback=selectFirstMediaCallback) {
	let current_pl = viewed ? viewedMediaSetProperties.values.actorUuidRole : inspectedMediaSetProperties.values.actorUuidRole

	addToPlaylist(indexes, current_pl, media_uuid, "Untitled Playlist", callback)
}

function addToNewPlaylist(indexes=[], media_uuid=null, callback=selectFirstMediaCallback) {
	addToPlaylist(indexes, null, media_uuid, "Untitled Playlist", callback)
}


function loadShotGridPlaylist(shotgrid_playlist_id, name, context={}) {

	// console.log("createPlaylist", name)
	let plindex = theSessionData.createPlaylist(name)

	// mark playlist as busy.
    plindex.model.set(plindex, true, "busyRole")

	// get playlist actor uuid
	let pl_actor_uuid = plindex.model.get(plindex, "actorUuidRole")

    // if(make_active){
    //     app_window.sessionFunction.setActivePlaylist(index)
    // }

	// get versions..
    Future.promise(ShotBrowserEngine.getPlaylistVersionsFuture(shotgrid_playlist_id)).then(function(json_string) {
        try {
            var data = JSON.parse(json_string)
            if(data["data"]){
            	// inject shotgrid json into playlist
                Future.promise(plindex.model.setJSONFuture(plindex, JSON.stringify(data['data']), "/metadata/shotgun/playlist")).then(
                    function(result) {
                        // addDecorator(pl_actor_uuid)
                        // addMenusFull(pl_actor_uuid)
                    },
                    function() {
                    }
                )

                data["context"] = context

                // add versions to playlist.
                Future.promise(ShotBrowserEngine.addVersionToPlaylistFuture(JSON.stringify(data), pl_actor_uuid)).then(
                    function(json_string) {
                    	var uuids = JSON.parse(json_string)
                    	if(uuids.length) {
                        	let tmp = []//uuids.length
                        	for(let i=0;i<1;i++)
	                        	tmp.push(helpers.QVariantFromUuidString(uuids[i]))

	                        mediaSelectionModel.selectNewMedia(plindex, tmp)
                        }

                        plindex.model.set(plindex, false, "busyRole")

						// selects the playlist so it is what's showing in
						// the viewport
						sessionSelectionModel.setCurrentIndex(plindex, ItemSelectionModel.ClearAndSelect)

                        // ShotgunHelpers.handle_response(json_string)
                    },
                    function() {
                        plindex.model.set(plindex, false, "busyRole")
                    }
                )
            } else {
                plindex.model.set(plindex, false, "busyRole")
                console.log("loadShotgridPlaylist", json_string)
            }
	        } catch(err) {
			    plindex.model.set(plindex, false, "busyRole")
    		// error.title = "Load ShotGrid Playlist " + name
    		// error.text = err + "\n" + json_string
    		// error.open()
    			console.log("loadShotgridPlaylist", err, json_string)
		}
	})
}

function loadShotgridPlaylists(indexes=[]) {
	indexes = mapIndexesToResultModel(indexes)

	if(indexes.length) {
		// console.log("loadShotgridPlaylists", indexes.length)
		let m = indexes[0].model
		for(let i=0; i<indexes.length; i++) {
			// console.log("loadShotgridPlaylists", m.get(indexes[i], "typeRole"))
			if(m.get(indexes[i], "typeRole") == "Playlist") {
				// create playlist
				let name = m.get(indexes[i], "nameRole")
		        let shotgrid_playlist_id = m.get(indexes[i], "idRole")
				loadShotGridPlaylist(shotgrid_playlist_id, name, m.context)
			}
		}
	}
}

function addToCurrent(indexes=[], viewed=true) {
	if(viewedMediaSetProperties.values.typeRole == "Timeline") {
		addToSequence(indexes, viewed)
	} else {
        addToCurrentPlaylist(indexes, null, viewed)
	}
}

function addSequencesToNewPlaylist(indexes=[], callback=selectTimelineCallback) {
	indexes = mapIndexesToResultModel(indexes)

	for(let i=0; i<indexes.length;i++) {
		let sequenceRole = indexes[i].model.get(indexes[i],"sequenceRole")
		addSequencesToPlaylist([indexes[i]], null, sequenceRole ? sequenceRole : "ShotBrowser Sequence", callback)
	}
}

function addSequencesToCurrentPlaylist(indexes=[], viewed=true, callback=selectTimelineCallback) {
	let current_pl = viewed ? viewedMediaSetProperties.index : inspectedMediaSetProperties.index

	addSequencesToPlaylist(indexes, current_pl, "ShotBrowser Sequence", callback)
}

function addSequencesToPlaylist(indexes, playlist_index=null, playlist_name ="ShotBrowser Sequence", callback=selectTimelineCallback) {
	indexes = mapIndexesToResultModel(indexes)

	if(indexes.length) {
		let m = indexes[0].model

		// console.log("playlist_index", playlist_index)

		for(let i =0; i <indexes.length; i++) {
			let path = m.get(indexes[i], "otioRole");
			if(helpers.urlExists(path)) {
				if(!playlist_index || !playlist_index.valid)
					playlist_index = theSessionData.createPlaylist(playlist_name)

		        Future.promise(theSessionData.handleDropFuture(Qt.CopyAction, {"text/uri-list": path}, playlist_index)).then(
		            function(quuids){
		            	callback(playlist_index, [quuids[0]])
		            },
			        function() {
			        }
		        )
		    } else {
		    	console.log("Path doesn't exist ", path)
		    }
		}
	}
}

function addToPlaylist(indexes=[], playlist_uuid=null, before_uuid=null, playlist_name = "Untitled Playlist", callback=null) {
	indexes = mapIndexesToResultModel(indexes)

	let shotgrid_playlists = []

	if(indexes.length) {
		if(before_uuid == null)
			before_uuid = helpers.QVariantFromUuidString("")

		let m = indexes[0].model

		let data = new Map()
		data["context"] = m.context
		data["data"] = []

		// our indexes might not be versions...
		//  we need to somehow get a version from the thing..

		for(let i=0; i<indexes.length; i++) {
			if(m.get(indexes[i], "typeRole") == "Version") {
				data["data"].push(m.get(indexes[i], "jsonRole"))
			} else if(m.get(indexes[i], "typeRole") == "Note") {
				let project_id = m.get(indexes[i],"projectIdRole")
				let version_ids = m.get(indexes[i], "linkedVersionsRole")
				if(version_ids.length && project_id) {
					let tmp = JSON.parse(ShotBrowserEngine.getVersions(project_id, version_ids))
					for(let j=0; j<tmp["data"].length; j++) {
						data["data"].push(tmp["data"][j])
					}
		        }
			} else if(m.get(indexes[i], "typeRole") == "Playlist") {
				shotgrid_playlists.push(indexes[i])
			}
		}

		if(data["data"].length) {

			// we call this for playlists.. and need special handling..
			if(playlist_uuid == null) {
				let plindex = theSessionData.createPlaylist(playlist_name)
		  	    playlist_uuid =  helpers.QVariantFromUuidString(plindex.model.get(plindex, "actorUuidRole"))
			}

	 	    Future.promise(ShotBrowserEngine.addVersionsToPlaylistFuture(data, playlist_uuid, before_uuid)).then(
		        function(json_string) {
	                try {
	                    var data = JSON.parse(json_string)
	                    callback(playlist_uuid, data)

	                    // app_window.sessionFunction.setActivePlaylist(index)
	                    // app_window.requestActivate()
	                    // app_window.raise()
	                    // sessionWidget.playerWidget.forceActiveFocus()

	                    // // add media uuid to selection and focus it.
	                    // let mind = sessionSelectionModel.model.search("{"+data[0]+"}", "actorUuidRole", sessionSelectionModel.model.index(0,0, index))
	                    // if(mind.valid)
	                    //     app_window.sessionFunction.setActiveMedia(mind)

	                } catch(err) {
	                    console.log(err)
	                    // ShotgunHelpers.handle_response(json_string)
	                }
		        },
		        function() {
		        }
		    )
	 	}

	 	if(shotgrid_playlists.length)
	 		loadShotgridPlaylists(shotgrid_playlists)
	}
}


function selectItem(selectionModel, index) {
	selectionModel.select(index, ItemSelectionModel.ClearAndSelect)
}

function ctrlSelectItem(selectionModel, index) {
	selectionModel.select(index, ItemSelectionModel.Toggle)
}

function shiftSelectItem(selectionModel, index) {
	let sel = selectionModel.selectedIndexes
	if(sel.length) {
		// find last selected entry ?
		let m = sel[sel.length-1]
		let s = Math.min(index.row, m.row)
		let e = Math.max(index.row, m.row)
		let items = []

		for(let i=s; i<=e; i++) {
			items.push(selectionModel.model.index(i, 0, index.parent))
		}
		selectionModel.select(helpers.createItemSelection(items), ItemSelectionModel.ClearAndSelect)
	} else {
		selectItem(selectionModel, index)
	}
}

function updateMetadata(enabled, mediaUuid) {
    if(enabled && ShotBrowserEngine.liveLinkKey != mediaUuid) {
        ShotBrowserEngine.liveLinkKey = mediaUuid

        let mindex = theSessionData.searchRecursive(mediaUuid, "actorUuidRole", theSessionData.index(0, 0))
        if(mindex.valid) {
            Future.promise(
                mindex.model.getJSONFuture(mindex, "", true)
            ).then(function(json_string) {
                ShotBrowserEngine.liveLinkKey = mediaUuid
                // inject current sources.

                let jsn = JSON.parse(json_string)

                try {
	                let image_actor_idx = mindex.model.searchRecursive(mindex.model.get(mindex, "imageActorUuidRole"), "actorUuidRole", mindex)
	                let audio_actor_idx = mindex.model.searchRecursive(mindex.model.get(mindex, "audioActorUuidRole"), "actorUuidRole", mindex)

	                if(image_actor_idx.valid) {
	                	jsn["metadata"]["image_source"] = image_actor_idx.model.get(image_actor_idx,"nameRole")
	                } else {
		                jsn["metadata"]["image_source"] = "movie_dneg"
	                }
	                if(audio_actor_idx.valid) {
	                	jsn["metadata"]["audio_source"] = audio_actor_idx.model.get(audio_actor_idx,"nameRole")
	                } else {
		                jsn["metadata"]["audio_source"] = "movie_dneg"
	                }
	            }catch(err){}
                ShotBrowserEngine.liveLinkMetadata = JSON.stringify(jsn)
            })
            return true
        } else {
			ShotBrowserEngine.liveLinkMetadata = ""
        }
    }
    return false;
}

function transfer(destination, indexes) {
    let project = null
    let source = null

    let uuids = []

	indexes = mapIndexesToResultModel(indexes)

    if(indexes.length) {
    	let model = indexes[0].model

	    for(let i=0; i < indexes.length; i++) {

	        let dnuuid = model.get(indexes[i].row,"stalkUuidRole")
	        if(project == null) {
	            project = model.get(indexes[i].row,"projectRole")
	        }
	        // where is it...
	        if(source == null) {
	            if(model.get(indexes[i].row,"onSiteLon") && destination != "lon")
	                source = "lon"
	            else if(model.get(indexes[i].row,"onSiteMum") && destination != "mum")
	                source = "mum"
	            else if(model.get(indexes[i].row,"onSiteVan") && destination != "van")
	                source = "van"
	            else if(model.get(indexes[i].row,"onSiteChn") && destination != "chn")
	                source = "chn"
	            else if(model.get(indexes[i].row,"onSiteMtl") && destination != "mtl")
	                source = "mtl"
	            else if(model.get(indexes[i].row,"onSiteSyd") && destination != "syd")
	                source = "syd"
	        }
	        uuids.push(dnuuid)
	    }

	    if(project && source && destination && uuids.length) {
	    	// console.log(project,source,destination,uuids)
		    var fa = ShotBrowserEngine.requestFileTransferFuture(uuids, project, source, destination)

		    Future.promise(fa).then(
		        function(result) {}
		    )
	    }
	}
}


function syncPlaylistFromShotGrid(playlistUuid, callback=console.log) {
    Future.promise(
        ShotBrowserEngine.refreshPlaylistVersionsFuture(playlistUuid)
    ).then(function(json_string) {
    	callback(json_string)
    })
}


function syncPlaylistToShotGrid(playlistUuid, callback=console.log) {
    Future.promise(
        ShotBrowserEngine.updatePlaylistVersionsFuture(playlistUuid)
    ).then(function(json_string) {
    	callback(json_string)
    })
}

function publishPlaylistToShotGrid(playlistUuid, projectId, name, location, playlistType, callback=console.log ) {
    Future.promise(
        ShotBrowserEngine.createPlaylistFuture(playlistUuid, projectId, name, location, playlistType)
    ).then(function(json_string) {
    	callback(json_string)
    })
}

function getValidMediaCount(playlistUuid, callback=console.log) {
    Future.promise(
        ShotBrowserEngine.getPlaylistLinkMediaFuture(playlistUuid)
    ).then(function(json_string) {
        Future.promise(
            ShotBrowserEngine.getPlaylistValidMediaCountFuture(playlistUuid)
        ).then(function(json_string) {
        	callback(json_string)
        })
    })
}