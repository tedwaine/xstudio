import xstudio.qml.models 1.0
import xStudioReskin 1.0
import QtQml.Models 2.14
import xstudio.qml.helpers 1.0

XsPopupMenu {

    id: timelineMenu
    visible: false
    menu_model_name: "timeline_track_menu_"

    property var panelContext: helpers.contextPanel(timelineMenu)
    property var theTimeline: panelContext.theTimeline
    property var timelineSelection: theTimeline.timelineSelection
    property var debugSetMenuPathPosition: debug_menu.setMenuPathPosition

    property var currentTrackIndex: timelineSelection.selectedIndexes.length ? timelineSelection.selectedIndexes[0] : null

    onVisibleChanged: visible && updateFlags()

    function updateFlags() {
        if(currentTrackIndex) {
            let m = currentTrackIndex.model
            disabledTrack.isChecked = !m.get(currentTrackIndex, "enabledRole")
            lockedTrack.isChecked= m.get(currentTrackIndex, "lockedRole")
        }
        debug_menu.setMenuPathPosition("Debug", 40)
    }

    XsMenuModelItem {
        text: qsTr("Rename Track...")
        menuPath: ""
        menuItemPosition: 2
        menuModelName: timelineMenu.menu_model_name
        property var currentIndex: null
        onActivated: {
            let indexes = timelineSelection.selectedIndexes
            for(let i=0;i<indexes.length; i++) {
                currentIndex = indexes[i]
                dialogHelpers.textInputDialog(
                    acceptResult,
                    "Rename Track",
                    "Enter Track Name.",
                    theSessionData.get(indexes[i], "nameRole"),
                    ["Cancel", "Rename"])
            }
        }

        function acceptResult(new_name, button) {
            if (button == "Rename") {
                theTimeline.setItemName(currentIndex, new_name)
            }
        }
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        text: qsTr("Duplicate Tracks")
        menuPath: ""
        menuItemPosition: 4
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            let indexes = timelineSelection.selectedIndexes
            for(let i=0;i<indexes.length; i++) {
                theSessionData.duplicateRows(indexes[i].row, 1, indexes[i].parent)
            }
        }
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        text: qsTr("Flatten Selected Tracks")
        menuPath: ""
        menuItemPosition: 6
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            theSessionData.bakeTimelineItems(timelineSelection.selectedIndexes, "Flatten Track")
            theTimeline.deleteItems(timelineSelection.selectedIndexes)
        }
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        text: qsTr("Insert Track Above")
        menuPath: ""
        menuItemPosition: 8
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            let index = timelineSelection.selectedIndexes[0]
            let type = index.model.get(index,"typeRole")
            theTimeline.addItem(type, index.parent, index.row + (type == "Audio Track"?1:0), type)
        }
        panelContext: timelineMenu.panelContext
    }



    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 10
        menuModelName: timelineMenu.menu_model_name
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 20
        menuModelName: timelineMenu.menu_model_name
    }

    XsFlagMenuInserter {
        text: qsTr("Set Track Colour")
        menuModelName: timelineMenu.menu_model_name
        menuPath: ""
        menuPosition: 22
        onFlagSet: theTimeline.flagItems(timelineSelection.selectedIndexes, flag == "#00000000" ? "": flag)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 24
        menuModelName: timelineMenu.menu_model_name
    }

    XsMenuModelItem {
        id: disabledTrack
        text: qsTr("Disable Tracks")
        menuItemType: "toggle"
        menuPath: ""
        menuItemPosition: 26
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            theTimeline.enableItems(timelineSelection.selectedIndexes, isChecked)
            isChecked = !isChecked
        }
        isChecked: false
        panelContext: timelineMenu.panelContext

    }

    XsMenuModelItem {
        id: lockedTrack
        text: qsTr("Lock Tracks")
        menuItemType: "toggle"
        menuPath: ""
        menuItemPosition: 28
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            theTimeline.lockItems(timelineSelection.selectedIndexes, !isChecked)
            isChecked = !isChecked
        }
        isChecked: false
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        text: qsTr("Move Track Up")
        menuPath: ""
        menuItemPosition: 30
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.moveItems(timelineSelection.selectedIndexes, -1)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        text: qsTr("Move Track Down")
        menuPath: ""
        menuItemPosition: 32
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.moveItems(timelineSelection.selectedIndexes, 1)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 34
        menuModelName: timelineMenu.menu_model_name
    }

    XsMenuModelItem {
        text: qsTr("Remove Selected Tracks")
        menuPath: ""
        menuItemPosition: 36
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.deleteItems(timelineSelection.selectedIndexes)
        panelContext: timelineMenu.panelContext
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 38
        menuModelName: timelineMenu.menu_model_name
    }

    XsMenuModelItem {
        id: debug_menu
        text: qsTr("Dump JSON")
        menuPath: "Debug"
        menuItemPosition: 0
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