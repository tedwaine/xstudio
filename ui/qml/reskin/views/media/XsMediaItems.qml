// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

import xstudio.qml.session 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0

import "./widgets"
import "./functions"

XsListView {

    id: mediaList
    model: mediaListModelData

    property var columns_model_index: null
    property var dragTargetIndex

    property real itemRowHeight: 0
    property real itemRowWidth: 0

    cacheBuffer: 80
    boundsBehavior: Flickable.StopAtBounds

    XsMediaListModelData {
        id: mediaListModelData
        delegate: chooser
    }

    property alias mediaListModelData: mediaListModelData

    PropertyAnimation{
        id: autoScrollAnimator
        target: mediaList
        property: "contentY"
        duration: 100
    }

    property var selection: mediaSelectionModel.selectedIndexes
    onSelectionChanged: {

        if (!visible) return
        for (var i in selection) {
            // cordinate of selected item
            let item = mediaList.itemAtIndex(selection[i].row)
            if (item) {
                var mid = mediaList.itemAtIndex(selection[i].row).y + mediaList.itemAtIndex(selection[i].row).height/2
                if (mid > contentY && mid < (contentY+mediaList.height)) return
            }
        }

        if (selection.length && mediaList.itemAtIndex(selection[0].row) != undefined) {
            var mid = mediaList.itemAtIndex(selection[0].row).y + mediaList.itemAtIndex(selection[0].row).height/2
            autoScrollAnimator.to = mid - mediaList.height/2
            autoScrollAnimator.running = true
        }
    }

    Rectangle{ id: resultsBg
        anchors.fill: parent
        color: XsStyleSheet.panelBgColor
        z: -1
    }

    DelegateChooser {

        id: chooser
        role: "typeRole"

        DelegateChoice {

            roleValue: "Media";

            XsMediaItemDelegate {
                width: itemRowWidth
                property var media_item_model_index: theSessionData.index(index, 0, mediaListModelData.rootIndex)
            }

        }
    }

    XsHotkeyArea {
        id: hotkey_area
        anchors.fill: parent
        context: "" + mediaList
        focus: true
    }

    XsMediaListMouseArea {
        id: mouseArea
        anchors.fill: parent
    }

    XsMediaListFunctions {
        id: functions
    }

    property var selectAll: functions.selectAll
    property var deselectAll: functions.deselectAll
    property var mediaIndexAfterRemoved: functions.mediaIndexAfterRemoved
    property var deleteSelected: functions.deleteSelected
    property var selectUp: functions.selectUp
    property var selectDown: functions.selectDown

    Loader {
        id: menu_loader
    }

    // This menu is built from a menu model that is maintained by xSTUDIO's
    // backend. We access the menu model by an id string 'menuModelName' that
    // will be set by the derived type
    Component {
        id: menuComponent
        XsMediaListContextMenu {
            menuContextData: [mediaListModelData.name, "DFLDAS", 1.0, [1,2,3,4]]
        }
    }

    function showMenu(mx, my) {
        if (menu_loader.item == undefined) {
            menu_loader.sourceComponent = menuComponent
        }
        repositionPopupMenu(
            menu_loader.item,
            mediaList,
            mx,
            my);
    }

    /**************************************************************

    HOTKEYS

    ****************************************************************/
    XsHotkey {
        id: select_all_hotkey
        sequence: "Ctrl+A"
        name: "Select All Media in Playlist"
        description: "Selects all the media in the playlist/subset"
        context: "" + mediaList
        onActivated: {
            functions.selectAll()
        }
    }

    XsHotkey {
        id: deselect_all_hotkey
        sequence: "Ctrl+D"
        name: "Deselect All Media"
        description: "De-selects all the media in the playlist/subset"
        context: "" + mediaList
        onActivated: {
            functions.deselectAll()
        }
    }

    XsHotkey {
        id: delete_selected
        sequence: "Delete"
        name: "Delete Selected Media"
        description: "Removes selected media from media list"
        context: "" + mediaList
        onActivated: {
            functions.deleteSelected()
        }
    }

    XsHotkey {
        sequence: "Shift+Up"
        name: "Add to selected media (upwards)"
        description: "Adds the media item immediately above the first selected media item."
        context: "" + mediaList
        onActivated: functions.selectUp()
    }

    XsHotkey {
        sequence: "Shift+Down"
        name: "Add to selected media (downwards)"
        description: "Adds the media item immediately below the last selected media item."
        context: "" + mediaList
        onActivated: functions.selectDown()
    }

    property alias select_all_hotkey: select_all_hotkey
    property alias deselect_all_hotkey: deselect_all_hotkey


    XsDragDropHandler {

        id: drag_drop_handler
        dragSourceName: "MediaList"
        dragData: mediaSelectionModel.selectedIndexes
    
        onDragged: {
            computeTargetDropIndex(mousePosition.y)
            autoScroll(mousePosition.y)
        }

        onDropped: {
            
            if (!dragTargetIndex) return

            if (dragSourceName == "MediaList") {
                // selection being dropped from a media list. 'data' should be
                // a list of model indeces

                // are these indeces from the same list as our list here?
                if (data.length && data[0].parent == mediaListModelData.rootIndex) {
                    // do a move rows
                    theSessionData.moveRows(
                        data,
                        dragTargetIndex.row,
                        dragTargetIndex.parent.parent
                        )
                    dragTargetIndex = undefined
                }
            }
        }
    
    }

    property alias drag_drop_handler: drag_drop_handler

    function isInSelection(idx) {
        return mediaList.itemAtIndex(idx).isSelected
    }

    function computeTargetDropIndex(dropCoordY) {

        if (dropCoordY < 0 || dropCoordY > height) {
            dragTargetIndex = undefined
            return
        }

        var idx = mediaList.indexAt(10, dropCoordY + contentY)
        if (idx != -1) {
            var y = mediaList.mapToItem(mediaList.itemAtIndex(idx), 10, dropCoordY).y
            if (y > itemRowHeight/2 && idx < (mediaList.count-1)) {
                idx = idx+1
            }

            // the index that we are going to drop items into cannot
            // be one of the selected items. Find the nearest unselected
            // index
            if (isInSelection(idx)) {
                var lidx = idx
                while (isInSelection(lidx)) {
                    lidx = lidx-1
                    if (!lidx) break
                }
                var hidx = idx
                while (isInSelection(hidx)) {
                    if (hidx = (mediaList.count-1)) break
                    hidx = hidx+1
                }

                if ((idx-lidx) < (hidx-idx)) {
                    idx = lidx
                } else {
                    idx = hidx
                }

            }
            dragTargetIndex = mediaList.itemAtIndex(idx).media_item_model_index
        } else {
            dragTargetIndex = undefined
        }

    }

    property var autoScrollVelocity: 200
    function autoScroll(mouseY) {
        if ((height-mouseY) < 0 || mouseY < 0) {
            scrollUp.cancel()
            scrollDown.cancel()
        } else if ((height-mouseY) < 60) {
            scrollUp.cancel()
            scrollDown.run()
        } else if (mouseY < 60) {
            scrollDown.cancel()
            scrollUp.run()
        } else if (scrollUp.running) {
            scrollUp.cancel()
        } else if (scrollDown.running) {
            scrollDown.cancel()
        }
    }

    SmoothedAnimation { 
        id: scrollDown
        target: mediaList;
        properties: "contentY"; 
        velocity: autoScrollVelocity 
        to: mediaList.count*itemRowHeight - mediaList.height + originY
        function cancel() {
            if (running) stop()
        }
        function run() {
            if (!running && mediaList.contentY < (mediaList.count*itemRowHeight - mediaList.height + originY)) start()
        }
    }

    SmoothedAnimation { 
        id: scrollUp
        target: mediaList;
        properties: "contentY"; 
        velocity: autoScrollVelocity 
        to: originY
        function cancel() {
            if (running) stop()
        }
        function run() {
            if (!running && mediaList.contentY > originY) start()
        }
    }

}