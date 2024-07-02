// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12

import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0
import ShotBrowser 1.0
import xStudioReskin 1.0
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
        text: "Shot Browser"
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 4
        menuModelName: "media_list_menu_"
    }

    XsMenuModelItem {
        text: "In ShotGrid..."
        menuPath: "Reveal Source"
        menuItemPosition: 10
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.revealMediaInShotgrid(menuContext.mediaSelection)
    }
    XsMenuModelItem {
        text: "In Ivy..."
        menuPath: "Reveal Source"
        menuItemPosition: 11
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.revealMediaInIvy(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "Download SG Movie"
        menuPath: ""
        menuItemPosition: 11
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.downloadMovies(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "Download SG Previews (missing)"
        menuPath: ""
        menuItemPosition: 11
        menuModelName: "media_list_menu_"
        onActivated: ShotBrowserHelpers.downloadMissingMovies(menuContext.mediaSelection)
    }

    XsMenuModelItem {
        text: "Publish ShotGrid Media Notes"
        menuPath: ""
        menuItemPosition: 11
        menuModelName: "media_list_menu_"
        onActivated: {
            publish_notes.show()
            publish_notes.publishFromMedia(menuContext.mediaSelection)
        }
    }

    XsMenuModelItem {
        text: "ShotGrid Playlist Notes"
        menuPath: "Publish"
        menuItemPosition: 1
        menuModelName: "main menu bar"
        onActivated: {
            publish_notes.show()
            publish_notes.publishFromPlaylist(helpers.QVariantFromUuidString(viewedMediaSetProperties.values.actorUuidRole))
        }
    }

    XsMenuModelItem {
        text: "Refresh Current Playlist"
        menuPath: "Publish"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QVariantFromUuidString(viewedMediaSetProperties.values.actorUuidRole)
        )
    }

    XsMenuModelItem {
        text: "Push Current Playlist"
        menuPath: "Publish"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: {
            sync_to_dialog.show()
            sync_to_dialog.playlistProperties = viewedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Publish Current Playlist"
        menuPath: "Publish"
        menuItemPosition: 2
        menuModelName: "main menu bar"
        onActivated: {
            publish_to_dialog.show()
            publish_to_dialog.playlistProperties = viewedMediaSetProperties
        }
    }

    XsMenuModelItem {
        menuItemType: "divider"
        text: "ShotBrowser"
        menuPath: ""
        menuItemPosition: 100
        menuModelName: "playlist_context_menu"
    }

    XsMenuModelItem {
        text: "Refresh Selected Playlist"
        menuPath: ""
        menuItemPosition: 101
        menuModelName: "playlist_context_menu"
        onActivated: ShotBrowserHelpers.syncPlaylistFromShotGrid(
            helpers.QVariantFromUuidString(inspectedMediaSetProperties.values.actorUuidRole)
        )
    }

    XsMenuModelItem {
        text: "Push Current Playlist"
        menuPath: ""
        menuItemPosition: 102
        menuModelName: "playlist_context_menu"
        onActivated: {
            sync_to_dialog.show()
            sync_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Publish Current Playlist"
        menuPath: ""
        menuItemPosition: 103
        menuModelName: "playlist_context_menu"
        onActivated: {
            publish_to_dialog.show()
            publish_to_dialog.playlistProperties = inspectedMediaSetProperties
        }
    }

    XsMenuModelItem {
        text: "Publish ShotGrid Playlist Notes"
        menuPath: ""
        menuItemPosition: 104
        menuModelName: "playlist_context_menu"
        onActivated: {
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
