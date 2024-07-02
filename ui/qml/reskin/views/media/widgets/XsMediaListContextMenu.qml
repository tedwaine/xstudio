// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.12

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.clipboard 1.0
import QuickFuture 1.0

XsPopupMenu {
    id: btnMenu
    visible: false
    menu_model_name: "media_list_menu_"

    property var panelContext: helpers.contextPanel(btnMenu)

    /**************************************************************

    Static Menu Items (most items in this menu are added dymanically
        from the backend - e.g. PlayheadActor, Viewport classes do this)

    ****************************************************************/
    Clipboard {
      id: clipboard
    }

    XsFileFunctions {
        id: file_functions
   }

    XsFlagMenuInserter {
        text: qsTr("Media Colour")
        panelContext: btnMenu.panelContext
        menuModelName: btnMenu.menu_model_name
        menuPath: ""
        menuPosition: 0.0
        onFlagSet: {
            let sindexs = mediaSelectionModel.selectedIndexes
            for(let i = 0; i< sindexs.length; i++) {
                theSessionData.set(sindexs[i], flag, "flagColourRole")
                if (flag_text)
                    theSessionData.set(sindexs[i], flag_text, "flagTextRole")
            }
        }
    }

    XsPreference {
        id: sessionLinkPrefix
        path: "/core/session/session_link_prefix"
    }

    XsMenuModelItem {
        text: "Select All"
        menuItemType: "button"
        menuPath: ""
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        hotkeyUuid: select_all_hotkey.uuid
        onActivated: {
            mediaList.selectAll()
        }
        panelContext: btnMenu.panelContext
    }

    // XsMenuModelItem {
    //     text: "TEST"
    //     menuPath: ""
    //     menuItemPosition: 2.5
    //     menuModelName: btnMenu.menu_model_name
    //     onActivated: {
    //         let mlf = 14
    //         let c = theSessionData.getTimelineVisibleClipIndexes(viewedMediaSetIndex, mediaSelectionModel.selectedIndexes[0], mlf)
    //         if(c.length)
    //             console.log(theSessionData.getTimelineFrameFromClip(c[0], mlf))
    //     }
    //     panelContext: btnMenu.panelContext
    // }

    XsMenuModelItem {
        text: "De-Select All"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: btnMenu.menu_model_name
        hotkeyUuid: deselect_all_hotkey.uuid
        onActivated: {
            mediaList.deselectAll()
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuItemPosition: 2.5
        menuPath: ""
        menuModelName: btnMenu.menu_model_name
    }


    XsMenuModelItem {
        text: "Playlist..."
        menuPath: "Add To New"
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            dialogHelpers.textInputDialog(
                function(name, button) {
                    if(button == "Move Media") {
                        let pl = theSessionData.createPlaylist(name)
                        theSessionData.moveRows(mediaSelectionModel.selectedIndexes, 0, pl)
                    } else if(button == "Copy Media") {
                        let pl = theSessionData.createPlaylist(name)
                        theSessionData.copyRows(mediaSelectionModel.selectedIndexes, 0, pl)
                    }
                },
                "Add To New Playlist",
                "Enter New Playlist Name",
                "Untitled Playlist",
                ["Cancel", "Move Media", "Copy Media"])
        }

        panelContext: btnMenu.panelContext
    }


    XsMenuModelItem {
        text: "Subset..."
        menuPath: "Add To New"
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            dialogHelpers.textInputDialog(
                function(name, button) {
                    if(button == "Add Media") {
                        let sub = theSessionData.createPlaylistChild(name, "Subset", theSessionData.getPlaylistIndex(mediaSelectionModel.selectedIndexes[0]))
                        theSessionData.copyRows(mediaSelectionModel.selectedIndexes, 0, sub)
                    }
                },
                "Add To New Subset",
                "Enter New Subset Name",
                "Untitled Subset",
                ["Cancel", "Add Media"])
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Sequence..."
        menuPath: "Add To New"
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            dialogHelpers.textInputDialog(
                function(name, button) {
                    if(button == "Add Media") {
                        let sub = theSessionData.createPlaylistChild(name, "Timeline", theSessionData.getPlaylistIndex(mediaSelectionModel.selectedIndexes[0]))
                        theSessionData.copyRows(mediaSelectionModel.selectedIndexes, 0, sub)

                        let tindex = theSessionData.index(2, 0, sub)
                        theSessionData.fetchMoreWait(tindex)
                        let sindex = theSessionData.index(0, 0, tindex)
                        let vindex = theSessionData.index(0, 0, sindex)
                        let aindex = theSessionData.index(1, 0, sindex)

                        for(let i = 0; i< mediaSelectionModel.selectedIndexes.length; i++) {
                            let mindex = mediaSelectionModel.selectedIndexes[i]
                            theSessionData.insertTimelineClip(i, vindex, mindex, "")
                            theSessionData.insertTimelineClip(i, aindex, mindex, "")
                        }
                    }
                },
                "Add To New Sequence",
                "Enter New Sequence Name",
                "Untitled Sequence",
                ["Cancel", "Add Media"])
        }
        panelContext: btnMenu.panelContext
    }


    XsMenuModelItem {
        menuPath: "Copy"
        text: "Selected File Names"
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            let result = mediaSelectionModel.getSelectedMediaUrl("pathShakeRole")
            for(let i =0;i<result.length;i++) {
                result[i] = helpers.pathFromURL(result[i])
                result[i] = result[i].substr(result[i].lastIndexOf("/")+1)
            }
            clipboard.text = result.join("\n")
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        menuPath: "Copy"
        text: "Selected File Paths"
        menuItemPosition: 2
        menuModelName: btnMenu.menu_model_name

        onActivated: {
            let result = mediaSelectionModel.getSelectedMediaUrl("pathShakeRole")
            for(let i =0;i<result.length;i++) {
                result[i] = helpers.pathFromURL(result[i])
            }

            clipboard.text = result.join("\n")
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        menuPath: "Copy"
        text: "Copy Selected To Email Link"
        menuItemPosition: 3
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            let name = encodeURIComponent(inspectedMediaSetProperties.values.nameRole)
            let prefix = "&" + name + "_media="
            let filenames = mediaSelectionModel.getSelectedMediaUrl()
            for(let i =0;i<filenames.length;i++) {
                filenames[i] = helpers.pathFromURL(filenames[i])
            }

            clipboard.text = sessionLinkPrefix.value + "xstudio://add_media?compare="+
                encodeURIComponent(currentPlayhead.compareMode)+"&playlist=" +
                name + prefix +
                filenames.join(prefix)
        }

        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        menuPath: "Copy"
        text: "Copy Selected To Shell Command"
        menuItemPosition: 4
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            let name = encodeURIComponent(inspectedMediaSetProperties.values.nameRole)
            let prefix = "&" + name + "_media="
            let filenames = mediaSelectionModel.getSelectedMediaUrl()
            for(let i =0;i<filenames.length;i++) {
                filenames[i] = helpers.pathFromURL(filenames[i])
            }

            clipboard.text = "xstudio 'xstudio://add_media?compare="+
                encodeURIComponent(currentPlayhead.compareMode)+"&playlist=" +
                name + prefix +
                filenames.join(prefix) +"'"
        }

        panelContext: btnMenu.panelContext

        Component.onCompleted: {
            // we need this so the menu model knows where to insert the
            // "Transfer Selected" sub menu in the top level menu
            setMenuPathPosition("Copy", 2.6)
        }
    }

    XsMenuModelItem {
        text: "Duplicate Media"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            let items = []

            for(let i=0;i<mediaSelectionModel.selectedIndexes.length;++i)
                items.push(helpers.makePersistent(mediaSelectionModel.selectedIndexes[i]))

            items.forEach(
                (item) => {
                    item.model.duplicateRows(item.row, 1, item.parent)
                }
            )
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Dump Metadata"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            console.log(theSessionData.getJSON(mediaList.selection[0],""))
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "On Disk..."
        menuPath: "Reveal Source"
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        onActivated: helpers.ShowURIS(mediaSelectionModel.getSelectedMediaUrl())
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Dump JSON"
        menuPath: ""
        menuItemPosition: 3.2
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            console.log(theSessionData.get(mediaList.selection[0], "jsonTextRole"))
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Delete Selected"
        menuPath: ""
        menuItemPosition: 100
        menuModelName: btnMenu.menu_model_name
        hotkeyUuid: delete_selected.uuid
        onActivated: {
            deleteSelected()
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Clear Cache"
        menuPath: "Advanced"
        menuItemPosition: 1
        menuModelName: btnMenu.menu_model_name
        onActivated: studio.clearImageCache()
        panelContext: btnMenu.panelContext

    }

    XsMenuModelItem {
        text: "Clear Selected Media From Cache"
        menuPath: "Advanced"
        menuItemPosition: 2
        menuModelName: btnMenu.menu_model_name
        onActivated: Future.promise(theSessionData.clearCacheFuture(mediaSelectionModel.selectedIndexes)).then(function(result){})
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Set Selected Media Rate"
        menuPath: "Advanced"
        menuItemPosition: 3
        menuModelName: btnMenu.menu_model_name
        onActivated: {
            let mi = mediaSelectionModel.selectedIndexes[0]
            let ms = theSessionData.searchRecursive(theSessionData.get(mi, "imageActorUuidRole"), "actorUuidRole", mi)

            dialogHelpers.numberInputDialog(
                function(new_rate, button) {
                    if(button == "Set Rate") {
                        mediaSelectionModel.selectedIndexes.forEach(
                            (element) => {
                                let mi = theSessionData.searchRecursive(theSessionData.get(element, "imageActorUuidRole"), "actorUuidRole", element)
                                mi.model.set(mi, new_rate, "rateFPSRole")
                            }
                        )
                    }
                },
                "Set Media Rate",
                "Enter New Media Rate",
                 mi.model.get(ms, "rateFPSRole"),
                ["Cancel", "Set Rate"])

        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Load Additional Sources For Selected Media"
        menuPath: "Advanced"
        menuItemPosition: 4
        menuModelName: btnMenu.menu_model_name
        onActivated: theSessionData.gatherMediaFor(theSessionData.getPlaylistIndex(mediaSelectionModel.selectedIndexes[0]), mediaSelectionModel.selectedIndexes)
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Relink Selected Media"
        menuPath: "Advanced"
        menuItemPosition: 5
        menuModelName: btnMenu.menu_model_name
        onActivated: {
           dialogHelpers.showFileDialogFolderMode(
                function(path, folder, chaserFunc) {
                    theSessionData.relinkMedia(mediaSelectionModel.selectedIndexes, path)
                },
                file_functions.defaultSessionFolder(),
                "Relink media files")
        }
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Decompose Selected Media"
        menuPath: "Advanced"
        menuItemPosition: 6
        menuModelName: btnMenu.menu_model_name
        onActivated: theSessionData.decomposeMedia(mediaSelectionModel.selectedIndexes)
        panelContext: btnMenu.panelContext
    }

    XsMenuModelItem {
        text: "Reload Selected Media"
        menuPath: "Advanced"
        menuItemPosition: 7
        menuModelName: btnMenu.menu_model_name
        onActivated: theSessionData.rescanMedia(mediaSelectionModel.selectedIndexes)
        panelContext: btnMenu.panelContext
    }
}
