// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQml.Models 2.15

import xStudioReskin 1.0

Item { 
    
    function selectAll() {
        var media_idx = theSessionData.index(0, 0, inspectedMediaSetIndex)
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
            "Remove the selected media?",
            ["Cancel", "Delete"],
            undefined)

    }

    function deleteSelectedCallback(response) {

        if (response != "Delete") return;

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

    function selectUp() {

        if (!mediaSelectionModel.selectedIndexes.length) return;
        var first_row = 10000000
        for (let i = 0; i < mediaSelectionModel.selectedIndexes.length; i++) {
            first_row = Math.min(mediaSelectionModel.selectedIndexes[i].row, first_row)
        }
        if (first_row > 0) {
            let parent = mediaSelectionModel.selectedIndexes[0].parent;
            mediaSelectionModel.select(
                parent.model.index(first_row-1, 0, parent),
                ItemSelectionModel.Select
                )
        }
    }

    function selectDown() {

        if (!mediaSelectionModel.selectedIndexes.length) return;
        var last_row = -1000000
        for (let i = 0; i < mediaSelectionModel.selectedIndexes.length; i++) {
            last_row = Math.max(mediaSelectionModel.selectedIndexes[i].row, last_row)
        }
        let parent = mediaSelectionModel.selectedIndexes[0].parent;
        if (last_row > 0 && parent.model.rowCount(parent) > (last_row+1)) {
            mediaSelectionModel.select(
                parent.model.index(last_row+1, 0, parent),
                ItemSelectionModel.Select
                )
        }
    }

}