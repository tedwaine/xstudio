// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQml.Models 2.15

import xStudio 1.0

Item {

    function toggleSelection() {

        var selectedMedia = mediaSelection
        var selected = false
        for (var i = 0; i < selectedMedia.length; ++i) {
            if (selectedMedia[i] == uuidRole) {
                selectedMedia.splice(i, 1)
                selected = true
                break;
            }
        }
        if (!selected) {
            selectedMedia.push(uuidRole)
        }
        if (selectedMedia.length > 0)
            mediaSelectionThingy.setMediaSelection(selectedMedia)

    }

    function exclusiveSelect() {

       mediaSelectionThingy.setMediaSelection([uuidRole])

    }

    function inclusiveSelect() {

        // For Shift key and select find the nearest selected row,
        // select items between that row and the row of THIS item

        // This is a bit awkward because this item lives in the 
        // filtered media list, but we need to use the full media
        // list model to use 'getData'

        var row = index // row of this item
        var d = 10000
        var nearest_row = -1

        let selection = mediaSelection

        let my_row = filteredmediaModelData.rowToSourceIndex(index).row

        // looping over media visible in the list
        for (var i = 0; i < filteredmediaModelData.rowCount(); ++i) {

            let uuid = filteredmediaModelData.getData(i, "uuidRole")
            console.log("uuid:", uuid, selection.indexOf(uuid))
            if (selection.indexOf(uuid) === -1)
                continue;
                 
            var delta = Math.abs(i-my_row)            
            if (delta < d) {
                d = delta
                nearest_row = i
            }

        }

        if (nearest_row!=-1) {

            var first = Math.min(my_row, nearest_row)
            var last = Math.max(my_row, nearest_row)

            for (var i = first; i <= last; ++i) {

                let uuid = filteredmediaModelData.getData(i, "uuidRole")
                if (selection.indexOf(uuid) === -1)
                    selection.push(uuid)

            }

            mediaSelectionThingy.setMediaSelection(selection)
        }
    }

}
