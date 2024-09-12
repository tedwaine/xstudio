// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12

import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0
import ShotBrowser 1.0
import xStudio 1.0
import xstudio.qml.helpers 1.0
import QuickFuture 1.0

Item {

    // Note: For each instance of the ShotBrowser panel, we will have an
    // instance of THIS item. As such, the 'menu_model_name' needs to be
    // unique for each instance, so it has its own model data in the backend
    // from which the actual menu instance (of which there will also be
    // multiple instances) is built. See ShotBrowserPanel

    // Create a menu 'Some Menu' with an item in it that says 'Do Something'

    XsMenuModelItem {
        text: "Pipeline"
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 25
        menuModelName: "media_list_menu_"
    }

    XsMenuModelItem {
        text: "In ShotGrid..."
        menuPath: "Reveal Source"
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
        text: "Publish Selected Media Notes"
        menuPath: ""
        menuItemPosition: 25.1
        menuModelName: "media_list_menu_"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_notes.show()
            publish_notes.publishFromMedia(menuContext.mediaSelection)
        }
    }

    XsMenuModelItem {
        text: "Download Missing SG Previews"
        menuPath: ""
        menuItemPosition: 26.1
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.downloadMissingMovies(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "Download SG Movie"
        menuPath: ""
        menuItemPosition: 26.2
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.downloadMovies(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "To Chennai"
        menuItemPosition: 1
        menuPath: "Transfer Selected|From London"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("lon", "chn", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Montreal"
        menuItemPosition: 3
        menuPath: "Transfer Selected|From London"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("lon", "mtl", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Mumbai"
        menuItemPosition: 4
        menuPath: "Transfer Selected|From London"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("lon", "mum", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Sydney"
        menuItemPosition: 5
        menuPath: "Transfer Selected|From London"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("lon", "syd", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Vancouver"
        menuItemPosition: 6
        menuPath: "Transfer Selected|From London"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("lon", "van", menuContext.mediaSelection)
    }


    XsMenuModelItem {
        text: "To London"
        menuItemPosition: 2
        menuPath: "Transfer Selected|From Chennai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("chn", "lon", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Montreal"
        menuItemPosition: 3
        menuPath: "Transfer Selected|From Chennai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("chn", "mtl", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Mumbai"
        menuItemPosition: 4
        menuPath: "Transfer Selected|From Chennai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("chn", "mum", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Sydney"
        menuItemPosition: 5
        menuPath: "Transfer Selected|From Chennai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("chn", "syd", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Vancouver"
        menuItemPosition: 6
        menuPath: "Transfer Selected|From Chennai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("chn", "van", menuContext.mediaSelection)
    }



    XsMenuModelItem {
        text: "To Chennai"
        menuItemPosition: 1
        menuPath: "Transfer Selected|From Montreal"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mtl", "chn", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To London"
        menuItemPosition: 2
        menuPath: "Transfer Selected|From Montreal"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mtl","lon", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Mumbai"
        menuItemPosition: 4
        menuPath: "Transfer Selected|From Montreal"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mtl","mum", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Sydney"
        menuItemPosition: 5
        menuPath: "Transfer Selected|From Montreal"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mtl","syd", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Vancouver"
        menuItemPosition: 6
        menuPath: "Transfer Selected|From Montreal"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mtl","van", menuContext.mediaSelection)
    }


    XsMenuModelItem {
        text: "To Chennai"
        menuItemPosition: 1
        menuPath: "Transfer Selected|From Mumbai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mum", "chn", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To London"
        menuItemPosition: 2
        menuPath: "Transfer Selected|From Mumbai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mum","lon", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Montreal"
        menuItemPosition: 3
        menuPath: "Transfer Selected|From Mumbai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mum","mtl", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Sydney"
        menuItemPosition: 5
        menuPath: "Transfer Selected|From Mumbai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mum","syd", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Vancouver"
        menuItemPosition: 6
        menuPath: "Transfer Selected|From Mumbai"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("mum","van", menuContext.mediaSelection)
    }




    XsMenuModelItem {
        text: "To Chennai"
        menuItemPosition: 1
        menuPath: "Transfer Selected|From Sydney"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("syd","chn", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To London"
        menuItemPosition: 2
        menuPath: "Transfer Selected|From Sydney"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("syd","lon", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Montreal"
        menuItemPosition: 3
        menuPath: "Transfer Selected|From Sydney"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("syd","mtl", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Mumbai"
        menuItemPosition: 4
        menuPath: "Transfer Selected|From Sydney"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("syd","mum", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Vancouver"
        menuItemPosition: 6
        menuPath: "Transfer Selected|From Sydney"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("syd","van", menuContext.mediaSelection)
    }


    XsMenuModelItem {
        text: "To Chennai"
        menuItemPosition: 1
        menuPath: "Transfer Selected|From Vancouver"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("van", "chn", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To London"
        menuItemPosition: 2
        menuPath: "Transfer Selected|From Vancouver"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("van", "lon", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Montreal"
        menuItemPosition: 3
        menuPath: "Transfer Selected|From Vancouver"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("van", "mtl", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Mumbai"
        menuItemPosition: 4
        menuPath: "Transfer Selected|From Vancouver"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("van", "mum", menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "To Sydney"
        menuItemPosition: 5
        menuPath: "Transfer Selected|From Vancouver"
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.transferMedia("van", "syd", menuContext.mediaSelection)
    }


    XsMenuModelItem {
        menuItemType: "divider"
        menuItemPosition: 7
        menuPath: "Transfer Selected"
        menuModelName: "media_list_menu_"
    }
    XsMenuModelItem {
        text: "Open Transfer Tool"
        menuItemPosition: 8
        menuPath: "Transfer Selected"
        menuModelName: "media_list_menu_"
        onActivated: helpers.startDetachedProcess("dnenv-do", [helpers.getEnv("SHOW"), helpers.getEnv("SHOT"), "--", "maketransfer"])

        Component.onCompleted: {
            // we need this so the menu model knows where to insert the
            // "Transfer Selected" sub menu in the top level menu
            setMenuPathPosition("Transfer Selected", 26.3)
        }
    }

    XsMenuModelItem {
        text: "Publish SG Playlist Notes"
        menuPath: "Pipeline"
        menuItemPosition: 1
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_notes.show()
            publish_notes.publishFromPlaylist(helpers.QVariantFromUuidString(viewedMediaSetProperties.values.actorUuidRole))
        }
    }

    XsMenuModelItem {
        text: "Reload Selected SG Playlists"
        menuPath: "Pipeline"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QVariantFromUuidString(viewedMediaSetProperties.values.actorUuidRole)
        )
    }

    XsMenuModelItem {
        text: "Push Media To Selected SG Playlists"
        menuPath: "Pipeline"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            sync_to_dialog.show()
            sync_to_dialog.playlistProperties = viewedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Publish New SG Playlist"
        menuPath: "Pipeline"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_to_dialog.show()
            publish_to_dialog.playlistProperties = viewedMediaSetProperties
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
        text: "Reload Selected SG Playlists"
        menuPath: ""
        menuItemPosition: 11
        menuModelName: "playlist_context_menu"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QVariantFromUuidString(inspectedMediaSetProperties.values.actorUuidRole)
        )
    }

    XsMenuModelItem {
        text: "Push Media To Selected SG Playlists"
        menuPath: ""
        menuItemPosition: 12
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            sync_to_dialog.show()
            sync_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Publish New SG Playlist"
        menuPath: ""
        menuItemPosition: 13
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_to_dialog.show()
            publish_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Publish SG Playlist Notes"
        menuPath: ""
        menuItemPosition: 14
        menuModelName: "playlist_context_menu"
        onActivated: {
            ShotBrowserEngine.connected = true
            publish_notes.show()
            publish_notes.publishFromPlaylist(helpers.QVariantFromUuidString(inspectedMediaSetProperties.values.actorUuidRole))
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
}
