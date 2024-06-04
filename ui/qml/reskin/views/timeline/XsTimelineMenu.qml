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
        menuPath: "Debug"
        menuItemPosition: 0.5
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


    // XsMenuModelItem {
    //     text: qsTr("Move Left")
    //     menuPath: "Adjust"
    //     menuItemPosition: 3
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: {
    //         if(timelineSelection.selectedIndexes.length) {
    //             theTimeline.moveItem(timelineSelection.selectedIndexes[0], -1)
    //         }
    //     }
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Move Right")
    //     menuPath: "Adjust"
    //     menuItemPosition: 4
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: {
    //         if(timelineSelection.selectedIndexes.length) {
    //             theTimeline.moveItem(timelineSelection.selectedIndexes[0], 1)
    //         }
    //     }
    //     panelContext: timelineMenu.panelContext
    // }


    // XsMenuModelItem {
    //     text: qsTr("Fit Selection")
    //     menuPath: ""
    //     menuItemPosition: 1
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.fitItems(timelineSelection.selectedIndexes)
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Jump to Start")
    //     menuPath: ""
    //     menuItemPosition: 4
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.jumpToStart()
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Jump to End")
    //     menuPath: ""
    //     menuItemPosition: 4
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.jumpToEnd()
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Align Left")
    //     menuPath: "Adjust"
    //     menuItemPosition: 5
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.leftAlignItems(timelineSelection.selectedIndexes)
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Align Right")
    //     menuPath: "Adjust"
    //     menuItemPosition: 6
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.rightAlignItems(timelineSelection.selectedIndexes)
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Move Range")
    //     menuPath: "Adjust"
    //     menuItemPosition: 7
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.moveItemFrames(timelineSelection.selectedIndexes[0], 0, 20, 40, true)
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Delete")
    //     menuPath: "Adjust"
    //     menuItemPosition: 8
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.deleteItems(timelineSelection.selectedIndexes)
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Delete Range")
    //     menuPath: "Adjust"
    //     menuItemPosition: 9
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.deleteItemFrames(timelineSelection.selectedIndexes[0], 10, 20)
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Undo")
    //     menuPath: "Adjust"
    //     menuItemPosition: 10
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.undo(viewedMediaSetProperties.index)
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Redo")
    //     menuPath: "Adjust"
    //     menuItemPosition: 11
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: theTimeline.redo(viewedMediaSetProperties.index)
    //     panelContext: timelineMenu.panelContext
    // }

   //  XsMenuModelItem {
   //      text: qsTr("Enable")
   //      menuPath: "State"
   //      menuItemPosition: 12
   //      menuModelName: timelineMenu.menu_model_name
   //      onActivated: theTimeline.enableItems(timelineSelection.selectedIndexes, true)
   //      panelContext: timelineMenu.panelContext
   //  }

   //  XsMenuModelItem {
   //      text: qsTr("Disable")
   //      menuPath: "State"
   //      menuItemPosition: 12
   //      menuModelName: timelineMenu.menu_model_name
   //      onActivated: theTimeline.enableItems(timelineSelection.selectedIndexes, false)
   //      panelContext: timelineMenu.panelContext
   //  }

   // XsMenuModelItem {
   //      text: qsTr("Lock")
   //      menuPath: "State"
   //      menuItemPosition: 12.5
   //      menuModelName: timelineMenu.menu_model_name
   //      onActivated: theTimeline.lockItems(timelineSelection.selectedIndexes, true)
   //      panelContext: timelineMenu.panelContext
   //  }

   //  XsMenuModelItem {
   //      text: qsTr("Unlock")
   //      menuPath: "State"
   //      menuItemPosition: 12.5
   //      menuModelName: timelineMenu.menu_model_name
   //      onActivated: theTimeline.lockItems(timelineSelection.selectedIndexes, false)
   //      panelContext: timelineMenu.panelContext
   //  }

   //  XsMenuModelItem {
   //      text: qsTr("Add Media")
   //      menuPath: "Adjust"
   //      menuItemPosition: 13
   //      menuModelName: timelineMenu.menu_model_name
   //      onActivated: theTimeline.addClip(
   //              timelineSelection.selectedIndexes[0].parent, timelineSelection.selectedIndexes[0].row,
   //              viewedMediaSetIndex
   //          )
   //      panelContext: timelineMenu.panelContext
   //  }

   //  XsMenuModelItem {
   //      text: qsTr("Add Gap")
   //      menuPath: "Adjust"
   //      menuItemPosition: 14
   //      menuModelName: timelineMenu.menu_model_name
   //      onActivated: theTimeline.addGap(timelineSelection.selectedIndexes[0].parent, timelineSelection.selectedIndexes[0].row)
   //      panelContext: timelineMenu.panelContext
   //  }

    // XsMenuModelItem {
    //     text: qsTr("Split")
    //     menuPath: "Adjust"
    //     menuItemPosition: 15
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: {
    //         if(timelineSelection.selectedIndexes.length) {
    //             let index = timelineSelection.selectedIndexes[0]
    //             theTimeline.splitClip(index, theSessionData.get(index, "trimmedStartRole") + (theSessionData.get(index, "trimmedDurationRole") /2))
    //         }
    //     }
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Duplicate")
    //     menuPath: "Adjust"
    //     menuItemPosition: 16
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: {
    //         let indexes = timelineSelection.selectedIndexes
    //         for(let i=0;i<indexes.length; i++) {
    //             theSessionData.duplicateRows(indexes[i].row, 1, indexes[i].parent)
    //         }
    //     }
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Change Name")
    //     menuPath: "State"
    //     menuItemPosition: 16
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: {
    //         let indexes = timelineSelection.selectedIndexes
    //         for(let i=0;i<indexes.length; i++) {
    //             set_name_dialog.index = indexes[i]
    //             set_name_dialog.text = theSessionData.get(indexes[i], "nameRole")
    //             set_name_dialog.open()
    //         }
    //     }
    //     panelContext: timelineMenu.panelContext
    // }

    // XsMenuModelItem {
    //     text: qsTr("Add Item")
    //     menuPath: "Adjust"
    //     menuItemPosition: 17
    //     menuModelName: timelineMenu.menu_model_name
    //     onActivated: {
    //         if(timelineSelection.selectedIndexes.length) {
    //             new_item_dialog.insertion_parent = timelineSelection.selectedIndexes[0].parent
    //             new_item_dialog.insertion_row = timelineSelection.selectedIndexes[0].row
    //         }
    //         else {
    //             new_item_dialog.insertion_parent = viewedMediaSetProperties.index
    //             new_item_dialog.insertion_row = 0
    //         }
    //         new_item_dialog.open()
    //     }
    //     panelContext: timelineMenu.panelContext
    // }
}