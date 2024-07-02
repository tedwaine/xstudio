import xstudio.qml.models 1.0
import xStudioReskin 1.0
import QtQml.Models 2.14

XsPopupMenu {

    id: timelineMenu
    visible: false
    menu_model_name: "timeline_clip_menu_"

    property var panelContext: helpers.contextPanel(timelineMenu)
    property var theTimeline: panelContext.theTimeline
    property var timelineSelection: theTimeline.timelineSelection

    property var debugSetMenuPathPosition: debug_menu.setMenuPathPosition

    property var currentClipIndex: timelineSelection.selectedIndexes.length ? helpers.makePersistent(timelineSelection.selectedIndexes[0]) : null

    onCurrentClipIndexChanged: {
        if(currentClipIndex && currentClipIndex.valid) {
            let m = currentClipIndex.model
            disabledClip.isChecked = !m.get(currentClipIndex, "enabledRole")
            lockedClip.isChecked= m.get(currentClipIndex, "lockedRole")
        }
    }

    function updateClipSelection(l,r) {
        timelineSelection.select(helpers.createItemSelection(
                theSessionData.modifyClipSelection(timelineSelection.selectedIndexes, l, r)
            ), ItemSelectionModel.ClearAndSelect)
    }

    XsMenuModelItem {
        text: "Select Next"
        menuPath: "Select"
        menuItemPosition: 1
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.select_next_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(-1, 1)
    }

    XsMenuModelItem {
        text: "Select Previous"
        menuPath: "Select"
        menuItemPosition: 2
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.select_previous_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(1, -1)
    }

    XsMenuModelItem {
        text: "Expand Next Selection"
        menuPath: "Select"
        menuItemPosition: 3
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.expand_next_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(0,+1)
    }

    XsMenuModelItem {
        text: "Expand Previous Selection"
        menuPath: "Select"
        menuItemPosition: 4
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.expand_previous_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(1, 0)
    }

    XsMenuModelItem {
        text: "Contract Next Selection"
        menuPath: "Select"
        menuItemPosition: 5
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.contract_next_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(0, -1)
    }

    XsMenuModelItem {
        text: "Contract Previous Selection"
        menuPath: "Select"
        menuItemPosition: 6
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.contract_previous_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(-1, 0)
    }

    XsMenuModelItem {
        text: "Expand Selection"
        menuPath: "Select"
        menuItemPosition: 7
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.expand_both_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(1, 1)
    }

    XsMenuModelItem {
        text: "Contract Selection"
        menuPath: "Select"
        menuItemPosition: 8
        menuModelName: timelineMenu.menu_model_name
        hotkeyUuid: theTimeline.contract_both_hotkey.uuid
        panelContext: timelineMenu.panelContext
        onActivated: updateClipSelection(-1, -1)
    }

    XsFlagMenuInserter {
        // panelContext: timelineMenu.panelContext
        text: qsTr("Media Colour")
        menuModelName: timelineMenu.menu_model_name
        menuPath: ""
        menuPosition: 2
        onFlagSet: {
            let sindexs = timelineSelection.selectedIndexes
            if(sindexs.length) {
                let m = sindexs[0].model
                let pindex = m.getPlaylistIndex(sindexs[0])
                let mlist = m.index(0, 0, pindex)

                for(let i = 0; i< sindexs.length; i++) {
                    let mediaIndex = m.search(m.get(sindexs[i],"clipMediaUuidRole"), "actorUuidRole", mlist)

                    m.set(mediaIndex, flag, "flagColourRole")
                    if (flag_text)
                        m.set(mediaIndex, flag_text, "flagTextRole")
                }
            }
        }
        panelContext: timelineMenu.panelContext
    }

    XsFlagMenuInserter {
        text: qsTr("Clip Colour")
        menuModelName: timelineMenu.menu_model_name
        menuPath: ""
        menuPosition: 3
        onFlagSet: theTimeline.flagItems(timelineSelection.selectedIndexes, flag == "#00000000" ? "": flag)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 4
        menuModelName: timelineMenu.menu_model_name
    }

    XsMenuModelItem {
        text: qsTr("Flatten To New Track")
        menuPath: ""
        menuItemPosition: 5
        menuModelName: timelineMenu.menu_model_name
        onActivated: theSessionData.bakeTimelineItems(timelineSelection.selectedIndexes)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        text: qsTr("Duplicate")
        menuPath: ""
        menuItemPosition: 6
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.duplicateClips(timelineSelection.selectedIndexes)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 7
        menuModelName: timelineMenu.menu_model_name
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 9
        menuModelName: timelineMenu.menu_model_name
    }


    XsMenuModelItem {
        id: disabledClip
        text: qsTr("Disable Clips")
        menuPath: ""
        menuItemType: "toggle"
        menuItemPosition: 10
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            theTimeline.enableItems(timelineSelection.selectedIndexes, isChecked)
            isChecked = !isChecked
        }
        isChecked: false
        panelContext: timelineMenu.panelContext
    }

   XsMenuModelItem {
        id: lockedClip
        text: qsTr("Lock Clips")
        menuPath: ""
        menuItemType: "toggle"
        menuItemPosition: 11
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            theTimeline.lockItems(timelineSelection.selectedIndexes, !isChecked)
            isChecked = !isChecked
        }
        isChecked: false
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 12
        menuModelName: timelineMenu.menu_model_name
    }


    XsMenuModelItem {
        text: qsTr("Remove Clips")
        menuPath: ""
        menuItemPosition: 13
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.deleteItems(timelineSelection.selectedIndexes)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        id: debug_menu
        text: qsTr("Dump JSON")
        menuPath: ""
        menuItemPosition: 14
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            for(let i=0;i<timelineSelection.selectedIndexes.length;i++) {
                console.log(timelineSelection.selectedIndexes[i])
                console.log(timelineSelection.selectedIndexes[i].model)
                console.log(timelineSelection.selectedIndexes[i].model.get(timelineSelection.selectedIndexes[i], "jsonTextRole"))
            }
        }
        panelContext: timelineMenu.panelContext
    }
}