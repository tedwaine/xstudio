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

XsListView {

    id: mediaList
    model: mediaListModelData

    property var columns_model: null

    property real itemRowHeight: 0
    property real itemRowWidth: 0
    cacheBuffer: 80

    XsMediaListModelData {
        id: mediaListModelData
        delegate: chooser
    }

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

            XsMediaSourceSelector {

                width: itemRowWidth
                height: itemRowHeight
                media_index_in_playlist: index
                media_item_model_index: theSessionData.index(index, 0, mediaListModelData.rootIndex)
                columns_model: mediaList.columns_model
            }
        }
    }

    XsHotkeyArea {
        id: hotkey_area
        anchors.fill: parent
        context: "" + mediaList
        focus: true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        property bool singleSelect: false
        onPressed: {
            if (mouse.button == Qt.RightButton) {
                showMenu(mouse.x, mouse.y)
            } else if (underMouseItem) {
                singleSelect = false
                if (mouse.modifiers == Qt.ControlModifier) {

                    underMouseItem.content.toggleSelection()

                } else if (mouse.modifiers == Qt.ShiftModifier) {

                    underMouseItem.content.inclusiveSelect()

                } else {

                    // behaviour should be similar to file browsers, for example.
                    // If you have multi-selection when you click on something that
                    // is already selected the selection does not change. When
                    // the mouse is released only the thing clicked on remains
                    // selected
                    if (underMouseItem.content.isSelected) {
                        singleSelect = true
                    } else {
                        underMouseItem.content.exclusiveSelect()
                    }

                }
            }
        }

        onReleased: {

            if (singleSelect) {
                underMouseItem.content.exclusiveSelect()
            }

        }

        onDoubleClicked: {
            if (underMouseItem) {
                underMouseItem.content.exclusiveSelect()
                // this sets the *viewed* playlist to match the playlist that this
                // media item belongs to so that this media item is shown in the
                // viewport
                viewedMediaSetIndex = mediaListModelData.rootIndex.parent
            }
        }

        property var underMouseItem
        onMouseYChanged: {
            var newUnderMouseItem = mediaList.itemAt(mouseArea.mouseX, mouseArea.mouseY + contentY)
            if (newUnderMouseItem != underMouseItem) {
                if (underMouseItem) underMouseItem.content.hovered = false
                underMouseItem = newUnderMouseItem
                if (underMouseItem) underMouseItem.content.hovered = true
            }
        }
        onContainsMouseChanged: {
            if (containsMouse) hotkey_area.forceActiveFocus()
        }

    }

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
        showPopupMenu(
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
            selectAll()
        }
    }

    XsHotkey {
        id: deselect_all_hotkey
        sequence: "Ctrl+D"
        name: "Deselect All Media"
        description: "De-selects all the media in the playlist/subset"
        context: "" + mediaList
        onActivated: {
            deselectAll()
        }
    }

    XsHotkey {
        id: delete_selected
        sequence: "Delete"
        name: "Delete Selected Media"
        description: "Removes selected media from media list"
        context: "" + mediaList
        onActivated: {
            deleteSelected()
        }
    }

    property alias select_all_hotkey: select_all_hotkey
    property alias deselect_all_hotkey: deselect_all_hotkey

    function selectAll() {
        var media_idx = theSessionData.index(0,0,selectedMediaSetIndex)
        var rc = theSessionData.rowCount(media_idx)
        var selection = []
        for (var i = 0; i < rc; ++i) {
            selection.push(theSessionData.index(i,0,media_idx))
        }
       mediaSelectionModel.select(
            helpers.createItemSelection(selection),
            ItemSelectionModel.ClearAndSelect
        )
    }

    function deselectAll() {
        mediaSelectionModel.clear()
    }

    function mediaIndexAfterRemoved(indexes) {

        let select_row = -1;
        let to_remove = []
        let parent = indexes[0].parent;

        for(let i =0; i<indexes.length; ++i)
            to_remove.push(indexes[i].row)

        to_remove = to_remove.sort(function(a,b){return a-b})

        while(select_row == -1 && to_remove.length) {
            select_row = to_remove[0] - 1
            to_remove.shift()
        }

        return parent.model.index(select_row, 0, parent)
    }

    function deleteSelected() {

        dialogHelpers.multiChoiceDialog(
            deleteSelectedCallback,
            "Delete Media",
            "Remove the selected meda?",
            ["Yes", "No"],
            undefined)

    }

    function deleteSelectedCallback(response) {

        if (response != "Yes") return;

        let items = []
        var l = mediaSelectionModel.selectedIndexes;
        for(let i=0;i<l.length;++i)
            items[i] = l[i]
        items = items.sort((a,b) => b.row - a.row )

        var onscreen_idx = mediaIndexAfterRemoved(items)
        mediaSelectionModel.setCurrentIndex(onscreen_idx, ItemSelectionModel.setCurrentIndex)
        mediaSelectionModel.select(onscreen_idx, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.setCurrentIndex)

        items.forEach(function (item, index) {
            item.model.removeRows(item.row, 1, false, item.parent)
        })
    }

}