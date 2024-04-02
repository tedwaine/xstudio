// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import xstudio.qml.viewport 1.0

Item {

    XsHotkey {
        sequence: ";"
        name: "Create new note"
        description: "Creates a new empty note on the current frame"
        context: "any"
        onActivated: {
            if(bookmarkModel.insertRows(bookmarkModel.rowCount(), 1)) {
                // set owner..
                let ind = bookmarkModel.index(bookmarkModel.rowCount()-1, 0)
                bookmarkModel.set(ind, currentOnScreenMediaData.values.actorUuidRole, "ownerRole")
                bookmarkModel.set(ind, currentPlayheadData.attributeRoleData("Position Seconds"), "startRole")
                bookmarkModel.set(ind, currentOnScreenMediaData.values.nameRole, "subjectRole")
                bookmarkModel.set(ind, 0, "durationRole")
                bookmarkModel.set(ind, preferences.note_category.value, "categoryRole")
                bookmarkModel.set(ind, preferences.note_colour.value, "colourRole")
            }
        }
    }
}