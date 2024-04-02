import xstudio.qml.models 1.0
import xStudioReskin 1.0
import QtQml.Models 2.14

XsPopupMenu {

    id: timelineMenu
    visible: false
    menu_model_name: "timeline_menu_"

    property var panelContext: helpers.contextPanel(timelineMenu)
    property var theTimeline: panelContext.theTimeline
    property var timelineSelection: theTimeline.timelineSelection
    property var timelineFocusSelection: theTimeline.timelineFocusSelection

    // XsMenusModel {
    //     id: timelineMenuModel

    //     // N.B. appending 'timelineMenu' means we have a unique menu model for each
    //     // instance of the XsTimelineMenu. This is important because we could
    //     // have multiple timeline instances, each with its own XsTimelineMenu...
    //     // Menu events like 'activated' are passed through the backend model
    //     // and therefore when they come back up to the UI layer, if we didn't
    //     // have unique models per timeline panel, the 'onActivated' would be
    //     // trigged on multiple instances of the correspondin XsMenuModelItem.
    //     modelDataName: "timelineMenu" + timelineMenu
    //     onJsonChanged: {
    //         timelineMenu.menu_model_index = index(-1, -1)
    //     }
    // }
    XsMenuModelItem {
        text: qsTr("Dump JSON")
        menuPath: ""
        menuItemPosition: 0.5
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            for(let i=0;i<timelineSelection.selectedIndexes.length;i++) {
                console.log(timelineSelection.selectedIndexes[i])
                console.log(timelineSelection.selectedIndexes[i].model)
                console.log(timelineSelection.selectedIndexes[i].model.get(timelineSelection.selectedIndexes[i], "jsonTextRole"))
            }
        }
    }


    XsMenuModelItem {
        text: qsTr("Set Focus")
        menuPath: ""
        menuItemPosition: 1
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            timelineFocusSelection.select(
                helpers.createItemSelection(timelineSelection.selectedIndexes),
                ItemSelectionModel.ClearAndSelect
            )
        }
    }

    XsMenuModelItem {
        text: qsTr("Clear Focus")
        menuPath: ""
        menuItemPosition: 2
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            timelineFocusSelection.clear()
        }
    }

    XsMenuModelItem {
        text: qsTr("Move Left")
        menuPath: ""
        menuItemPosition: 3
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            if(timelineSelection.selectedIndexes.length) {
                theTimeline.moveItem(timelineSelection.selectedIndexes[0], -1)
            }
        }
    }

    XsMenuModelItem {
        text: qsTr("Move Right")
        menuPath: ""
        menuItemPosition: 4
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            if(timelineSelection.selectedIndexes.length) {
                theTimeline.moveItem(timelineSelection.selectedIndexes[0], 1)
            }
        }
    }

    XsMenuModelItem {
        text: qsTr("Jump to Start")
        menuPath: ""
        menuItemPosition: 4
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.jumpToStart()
    }

    XsMenuModelItem {
        text: qsTr("Jump to End")
        menuPath: ""
        menuItemPosition: 4
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.jumpToEnd()
    }

    XsMenuModelItem {
        text: qsTr("Align Left")
        menuPath: ""
        menuItemPosition: 5
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.leftAlignItems(timelineSelection.selectedIndexes)
    }

    XsMenuModelItem {
        text: qsTr("Align Right")
        menuPath: ""
        menuItemPosition: 6
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.rightAlignItems(timelineSelection.selectedIndexes)
    }

    XsMenuModelItem {
        text: qsTr("Move Range")
        menuPath: ""
        menuItemPosition: 7
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.moveItemFrames(timelineSelection.selectedIndexes[0], 0, 20, 40, true)
    }

    XsMenuModelItem {
        text: qsTr("Delete")
        menuPath: ""
        menuItemPosition: 8
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.deleteItems(timelineSelection.selectedIndexes)
    }

    XsMenuModelItem {
        text: qsTr("Delete Range")
        menuPath: ""
        menuItemPosition: 9
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.deleteItemFrames(timelineSelection.selectedIndexes[0], 10, 20)
    }

    XsMenuModelItem {
        text: qsTr("Undo")
        menuPath: ""
        menuItemPosition: 10
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.undo(viewedMediaSetProperties.index)
    }

    XsMenuModelItem {
        text: qsTr("Redo")
        menuPath: ""
        menuItemPosition: 11
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.redo(viewedMediaSetProperties.index)
    }

    XsMenuModelItem {
        text: qsTr("Enable")
        menuPath: ""
        menuItemPosition: 12
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.enableItems(timelineSelection.selectedIndexes, true)
    }

    XsMenuModelItem {
        text: qsTr("Disable")
        menuPath: ""
        menuItemPosition: 12
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.enableItems(timelineSelection.selectedIndexes, false)
    }

    XsMenuModelItem {
        text: qsTr("Add Media")
        menuPath: ""
        menuItemPosition: 13
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.addClip(
                timelineSelection.selectedIndexes[0].parent, timelineSelection.selectedIndexes[0].row,
                viewedMediaSetIndex
            )
    }

    XsMenuModelItem {
        text: qsTr("Add Gap")
        menuPath: ""
        menuItemPosition: 14
        menuModelName: timelineMenu.menu_model_name
        onActivated: theTimeline.addGap(timelineSelection.selectedIndexes[0].parent, timelineSelection.selectedIndexes[0].row)
    }

    XsMenuModelItem {
        text: qsTr("Split")
        menuPath: ""
        menuItemPosition: 15
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            if(timelineSelection.selectedIndexes.length) {
                let index = timelineSelection.selectedIndexes[0]
                theTimeline.splitClip(index, theSessionData.get(index, "trimmedStartRole") + (theSessionData.get(index, "trimmedDurationRole") /2))
            }
        }
    }

    XsMenuModelItem {
        text: qsTr("Duplicate")
        menuPath: ""
        menuItemPosition: 16
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            let indexes = timelineSelection.selectedIndexes
            for(let i=0;i<indexes.length; i++) {
                theSessionData.duplicateRows(indexes[i].row, 1, indexes[i].parent)
            }
        }
    }

    XsMenuModelItem {
        text: qsTr("Change Name")
        menuPath: ""
        menuItemPosition: 16
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            let indexes = timelineSelection.selectedIndexes
            for(let i=0;i<indexes.length; i++) {
                set_name_dialog.index = indexes[i]
                set_name_dialog.text = theSessionData.get(indexes[i], "nameRole")
                set_name_dialog.open()
            }
        }
    }

    XsMenuModelItem {
        text: qsTr("Add Item")
        menuPath: ""
        menuItemPosition: 17
        menuModelName: timelineMenu.menu_model_name
        onActivated: {
            if(timelineSelection.selectedIndexes.length) {
                new_item_dialog.insertion_parent = timelineSelection.selectedIndexes[0].parent
                new_item_dialog.insertion_row = timelineSelection.selectedIndexes[0].row
            }
            else {
                new_item_dialog.insertion_parent = viewedMediaSetProperties.index
                new_item_dialog.insertion_row = 0
            }
            new_item_dialog.open()
        }
    }
}