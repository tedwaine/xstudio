// SPDX-License-Identifier: Apache-2.0
import QtQuick

import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0
import ShotBrowser 1.0
import xStudio 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.clipboard 1.0
import QuickFuture 1.0

Item {

    Clipboard {
      id: clipboard
    }

    // Note: For each instance of the ShotBrowser panel, we will have an
    // instance of THIS item. As such, the 'menu_model_name' needs to be
    // unique for each instance, so it has its own model data in the backend
    // from which the actual menu instance (of which there will also be
    // multiple instances) is built. See ShotBrowserPanel

    // Create a menu 'Some Menu' with an item in it that says 'Do Something'

    XsPreference {
       id: fullTransfer
       path: "/plugin/data_source/shotbrowser/transfer/full"
    }

    XsPreference {
       id: transferLeafs
       path: "/plugin/data_source/shotbrowser/transfer/leafs"
    }

    XsPreference {
        id: projectPref
        path: "/plugin/data_source/shotbrowser/browser/project"
    }


    property var leaves: fullTransfer.value ? [] : transferLeafs.value

    function getOffline() {
        var selection = []

        for (var i = 0; i < appWindow.mediaListModelData.rowCount(); ++i) {
            let si = appWindow.mediaListModelData.rowToSourceIndex(i)
            let state = theSessionData.get(si, "mediaStatusRole")
            if(state != undefined && state != "Online") {
                theSessionData.fetchMoreWait(si)
                selection.push(si)
            }
        }


        appWindow.mediaSelectionModel.select(
            helpers.createItemSelection(selection),
            ItemSelectionModel.ClearAndSelect
        )

        return selection
    }


    XsHotkey {
        id: reload_playlist
        sequence: "Alt+r"
        name: "Reload Playlist"
        description: "Reload Playlist From ShotGrid Ordered"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QUuidFromUuidString(inspectedMediaSetProperties.values.actorUuidRole), true
        )
        componentName: "ShotBrowser"
    }

    XsHotkey {
        id: qc_offline_current
        name: "Quick Cache Offline - Current"
        description: "Quick Cache Offline media"
        onActivated: ShotBrowserHelpers.useCache(getOffline())
        componentName: "Media List"
    }

    XsHotkey {
        id: qc_selected_current
        name: "Quick Cache Selected - Current"
        description: "Quick Cache Selected media"
        onActivated: ShotBrowserHelpers.useCache(mediaSelectionModel.selectedIndexes)
        componentName: "Media List"
    }

    XsMenuModelItem {
        text: "Pipeline"
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 200
        menuModelName: "media_list_menu_"
    }

    XsMenuModelItem {
        text: "In ShotGrid..." + (enabled ? "" : " (Production Only)")
        menuPath: "Reveal Source"
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuItemPosition: 2
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.revealMediaInShotgrid(menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "In Ivy..."
        menuPath: "Reveal Source"
        menuItemPosition: 3
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.revealMediaInIvy(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "Publish Media Notes..." + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Publish"
        menuItemPosition: 2
        menuModelName: "media_list_menu_"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_notes.show()
            publish_notes.publishFromMedia(menuContext.mediaSelection)
        }
        Component.onCompleted: {
            // we need this so the menu model knows where to insert the
            // "Transfer" sub menu in the top level menu
            setMenuPathPosition("Publish", 210)
        }

    }

    // XsMenuModelItem {
    //     text: "Download Missing SG Previews"
    //     menuPath: ""
    //     menuItemPosition: 26.1
    //     menuModelName: "media_list_menu_"
    //     onActivated: ShotBrowserHelpers.downloadMissingMovies(menuContext.mediaSelection)
    // }

    XsMenuModelItem {
        text: "Refresh SG Metadata"
        menuPath: ""
        menuItemPosition: 260
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.refreshMetadata(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "Download SG Movie"
        menuPath: "Media Actions"
        menuItemPosition: 261
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.downloadMovies(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "Quick Cache Offline"
        menuPath: ""
        hotkeyUuid: qc_offline_current.uuid
        menuItemPosition: 261
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(getOffline())
    }

    XsMenuModelItem {
        text: "Current"
        menuPath: "Quick Cache|Selected"
        hotkeyUuid: qc_selected_current.uuid
        menuItemPosition: 1
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "movie_dneg"
        menuPath: "Quick Cache|Selected"
        menuItemPosition: 2
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(menuContext.mediaSelection, text)
    }

    XsMenuModelItem {
        text: "client_movie"
        menuPath: "Quick Cache|Selected"
        menuItemPosition: 3
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(menuContext.mediaSelection, text)
    }

    XsMenuModelItem {
        text: "review_proxy_1"
        menuPath: "Quick Cache|Selected"
        menuItemPosition: 4
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(menuContext.mediaSelection, text)
    }

    XsMenuModelItem {
        text: "review_proxy_2"
        menuPath: "Quick Cache|Selected"
        menuItemPosition: 5
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(menuContext.mediaSelection, text)
    }

   XsMenuModelItem {
        text: "main_proxy0"
        menuPath: "Quick Cache|Selected"
        menuItemPosition: 6
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(menuContext.mediaSelection, text)
        Component.onCompleted: setMenuPathPosition("Quick Cache", 262)
    }

    XsMenuModelItem {
        text: "Current"
        hotkeyUuid: qc_offline_current.uuid
        menuPath: "Quick Cache|Offline"
        menuItemPosition: 1
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(getOffline())
    }

    XsMenuModelItem {
        text: "movie_dneg"
        menuPath: "Quick Cache|Offline"
        menuItemPosition: 2
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(getOffline(), text)
    }

    XsMenuModelItem {
        text: "client_movie"
        menuPath: "Quick Cache|Offline"
        menuItemPosition: 3
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(getOffline(), text)
    }

    XsMenuModelItem {
        text: "review_proxy_1"
        menuPath: "Quick Cache|Offline"
        menuItemPosition: 4
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(getOffline(), text)
    }

    XsMenuModelItem {
        text: "review_proxy_2"
        menuPath: "Quick Cache|Offline"
        menuItemPosition: 5
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(getOffline(), text)
    }

   XsMenuModelItem {
        text: "main_proxy0"
        menuPath: "Quick Cache|Offline"
        menuItemPosition: 6
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.useCache(getOffline(), text)
        Component.onCompleted: setMenuPathPosition("Quick Cache", 262)
    }


    XsMenuModelItem {
        text: "True"
        menuItemPosition: 1
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Set Status|Is Hero"+ (enabled ? "" : " (Production Only)")
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.markAsHero(menuContext.mediaSelection, true)
    }

    XsMenuModelItem {
        text: "False"
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuItemPosition: 2
        menuPath: "Set Status|Is Hero"+ (enabled ? "" : " (Production Only)")
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.markAsHero(menuContext.mediaSelection, false)
        Component.onCompleted: {
            setMenuPathPosition("Set Status|Is Hero"+ (enabled ? "" : " (Production Only)"), 3)
        }
    }

    XsMenuModelItem {
        text: "To Here"
        menuItemPosition: 1
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia(helpers.getEnv("DNSITEDATA_SHORT_NAME"), menuContext.mediaSelection, leaves)
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuItemPosition: 1.5
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
    }

    XsMenuModelItem {
        text: "To Chennai"
        menuItemPosition: 2
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("chn", menuContext.mediaSelection, leaves)
    }
    XsMenuModelItem {
        text: "To Montreal"
        menuItemPosition: 3
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mtl", menuContext.mediaSelection, leaves)
    }
    XsMenuModelItem {
        text: "To Mumbai"
        menuItemPosition: 4
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mum", menuContext.mediaSelection, leaves)
    }
    XsMenuModelItem {
        text: "To London"
        menuItemPosition: 4
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("lon", menuContext.mediaSelection, leaves)
    }
    XsMenuModelItem {
        text: "To Sydney"
        menuItemPosition: 5
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("syd", menuContext.mediaSelection, leaves)
    }
    // XsMenuModelItem {
    //     text: "To Vancouver"
    //     menuItemPosition: 6
    //     menuPath: "Transfer"
    //     menuModelName: "media_list_menu_"
    //     onActivated: ShotBrowserHelpers.transferMedia("van", menuContext.mediaSelection)
    // }

    XsMenuModelItem {
        menuItemType: "divider"
        menuItemPosition: 7
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
    }
    XsMenuModelItem {
        text: "Open Transfer Tool"
        menuItemPosition: 8
        menuPath: "Transfer"
        menuModelName: "media_list_menu_"
        onActivated: {
            let uuids = []
            if(menuContext.mediaSelection.length) {
                // get stalk uuids..
                let m = menuContext.mediaSelection[0].model
                for(let i =0; i< menuContext.mediaSelection.length; i++) {
                    let meta = JSON.parse(theSessionData.getJSON(menuContext.mediaSelection[i], "/metadata/shotgun/version/attributes/sg_ivy_dnuuid"))
                    if(meta)
                        uuids.push(meta)
                }
            }

            helpers.startDetachedProcess("dnenv-do", [helpers.getEnv("SHOW"), "--", "maketransfer"].concat(uuids))
        }

        Component.onCompleted: {
            // we need this so the menu model knows where to insert the
            // "Transfer" sub menu in the top level menu
            setMenuPathPosition("Transfer", 220)
        }
    }

    XsMenuModelItem {
        text: "Create SG Playlist..." + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Pipeline|ShotGrid Playlists"
        menuItemPosition: 1
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_to_dialog.show()
            publish_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
        Component.onCompleted: {
            helpers.setMenuPathPosition("Pipeline", "main menu bar", 3.0)
        }

    }

    XsMenuModelItem {
        text: "Create Reference Playlists"
        menuPath: "Pipeline|Reference"
        menuItemPosition: 1
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            let m = ShotBrowserEngine.presetsModel.termModel("Project")
            ShotBrowserHelpers.createReferencePlaylists(
                m.get(
                    m.searchRecursive(projectPref.value, "nameRole"),
                    "idRole"
                )
            )
        }
    }


    XsMenuModelItem {
        text: "Add SG Playlist from Clipboard"
        menuPath: "Pipeline|ShotGrid Playlists"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: {
            let result = /.*\/Playlist\/(\d+).*/.exec(clipboard.text)
            ShotBrowserHelpers.loadShotGridPlaylist(result[1])
        }
    }

    XsMenuModelItem {
        text: "Reload SG Playlist"
        menuPath: "Pipeline|ShotGrid Playlists"
        menuItemPosition: 2.1
        menuModelName: "main menu bar"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QUuidFromUuidString(inspectedMediaSetProperties.values.actorUuidRole)
        )
    }

    XsMenuModelItem {
        text: "Reload SG Playlist (Ordered)"
        // enabled: false
        menuPath: "Pipeline|ShotGrid Playlists"
        menuItemPosition: 2.5
        menuModelName: "main menu bar"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QUuidFromUuidString(inspectedMediaSetProperties.values.actorUuidRole),
            true
        )
    }

    XsMenuModelItem {
        text: "Push Media To SG Playlist" + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Pipeline|ShotGrid Playlists"
        menuItemPosition: 3
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            sync_to_dialog.show()
            sync_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Reveal In ShotGrid..." + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Pipeline|ShotGrid Playlists"
        menuItemPosition: 4
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            ShotBrowserHelpers.revealPlaylistInShotgrid(sessionSelectionModel.selectedIndexes)
        }
    }

    XsMenuModelItem {
        text: "Publish Playlist Notes" + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Pipeline|Notes"
        menuItemPosition: 1
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_notes.show()
            publish_notes.publishFromPlaylist(helpers.QVariantFromUuidString(inspectedMediaSetProperties.values.actorUuidRole))
        }
    }

    XsMenuModelItem {
        text: "Publish Selected Media Notes" + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Pipeline|Notes"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_notes.show()
            publish_notes.publishFromMedia(mediaSelectionModel.selectedIndexes)
        }
    }


    XsMenuModelItem {
        menuItemType: "divider"
        text: "Pipeline"
        menuPath: ""
        menuItemPosition: 10
        menuModelName: "playlist_context_menu"
    }


    XsMenuModelItem {
        text: "Create SG Playlist..." + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "ShotGrid Playlists"
        menuItemPosition: 1
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_to_dialog.show()
            publish_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
        Component.onCompleted: {
            setMenuPathPosition("ShotGrid Playlists", 10.1)
        }
    }

    XsMenuModelItem {
        text: "Add SG Playlist from Clipboard"
        menuPath: "ShotGrid Playlists"
        menuItemPosition: 2
        menuModelName: "playlist_context_menu"
        onActivated: {
            let result = /.*\/Playlist\/(\d+).*/.exec(clipboard.text)
            ShotBrowserHelpers.loadShotGridPlaylist(result[1])
        }
    }

    XsMenuModelItem {
        text: "Reload SG Playlist"
        menuPath: "ShotGrid Playlists"
        menuItemPosition: 2
        menuModelName: "playlist_context_menu"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QUuidFromUuidString(inspectedMediaSetProperties.values.actorUuidRole)
        )
    }

    XsMenuModelItem {
        text: "Reload SG Playlist (Ordered)"
        // enabled: false
        menuPath: "ShotGrid Playlists"
        menuItemPosition: 2.5
        menuModelName: "playlist_context_menu"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QUuidFromUuidString(inspectedMediaSetProperties.values.actorUuidRole),
            true
        )
        hotkeyUuid: reload_playlist.uuid
    }


    XsMenuModelItem {
        text: "Push Media To SG Playlist" + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "ShotGrid Playlists"
        menuItemPosition: 3
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            sync_to_dialog.show()
            sync_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Reveal In ShotGrid..." + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "ShotGrid Playlists"
        menuItemPosition: 4
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            ShotBrowserHelpers.revealPlaylistInShotgrid(sessionSelectionModel.selectedIndexes)
        }
    }

    XsMenuModelItem {
        text: "Create Reference Playlists"
        menuPath: "Reference"
        menuItemPosition: 1
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            let m = ShotBrowserEngine.presetsModel.termModel("Project")
            ShotBrowserHelpers.createReferencePlaylists(
                m.get(
                    m.searchRecursive(projectPref.value, "nameRole"),
                    "idRole"
                )
            )
        }
        Component.onCompleted: {
            setMenuPathPosition("Reference", 10.01)
        }
    }

    XsMenuModelItem {
        text: "SG Playlist Link "
        menuPath: "Copy To Clipboard"
        menuItemPosition: 4
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            clipboard.text = ShotBrowserHelpers.resolvePlaylistLink(sessionSelectionModel.selectedIndexes).join("\n")
        }
    }

    XsMenuModelItem {
        text: "Publish Playlist Notes" + (enabled ? "" : " (Production Only)")
        enabled: ShotBrowserEngine.shotGridLoginAllowed
        menuPath: "Notes"
        menuItemPosition: 1
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_notes.show()
            publish_notes.publishFromPlaylist(helpers.QVariantFromUuidString(inspectedMediaSetProperties.values.actorUuidRole))
        }
        Component.onCompleted: {
            setMenuPathPosition("Notes", 10.2)
        }
    }

    XsSBPublishNotesDialog {
        id: publish_notes
        property real btnHeight: XsStyleSheet.widgetStdHeight + 4
    }

    XsSBSyncPlaylistToShotGridDialog {
        id: sync_to_dialog
        width: 350
        height: 150
    }

    XsSBPublishPlaylistToShotGridDialog {
        id: publish_to_dialog
        width: 500
        height: 350
    }

    // This is required to create the application XsConformTool instance that
    // adds some default conform menus
    Component.onCompleted: {
        appWindow.createConformTool()
    }

}
