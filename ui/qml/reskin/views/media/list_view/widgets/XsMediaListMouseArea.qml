// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15

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
        
            forceActiveFocus()
            singleSelect = false
            if (mouse.modifiers == Qt.ControlModifier) {

                underMouseItem.toggleSelection()

            } else if (mouse.modifiers == Qt.ShiftModifier) {

                underMouseItem.inclusiveSelect()

            } else {

                // behaviour should be similar to file browsers, for example.
                // If you have multi-selection when you click on something that
                // is already selected the selection does not change. When
                // the mouse is released only the thing clicked on remains
                // selected
                if (underMouseItem.isSelected) {
                    singleSelect = true
                } else {
                    underMouseItem.exclusiveSelect()
                }

            }

            if (underMouseItem.playOnClick) {
                // playOnClick is true for the item if the mouse is 
                // over the play overlay in the thumbnail ... if this is
                // the case we set the playhead to the 'inspectedMediaSet'
                // so that we switch to viewing the media in this media
                // set.
                // The Timline test tells us to use the 'Aux Playhead' when
                // we are inspecting a timeline. The Aux Playhead lets us
                // view individual clips in a timeline, whereas the regular
                // playhead (from a timeline) only plays the whole timeline
                // itself.
                viewedMediaSetIndex = inspectedMediaSetIndex
                if (viewedMediaSetIndex.valid) {
                    theSessionData.setPlayheadTo(
                        viewedMediaSetIndex,
                        inspectedMediaSetProperties.values.typeRole == "Timeline")
                }

            }

            if (underMouseItem) {
                drag_drop_handler.startDrag(mouseX, mouseY)
            }
            
        }
        
    }

    onPositionChanged: {
        if (pressed && mediaSelectionModel.selectedIndexes.length) {
            interactive = false
            drag_drop_handler.doDrag(mouse.x, mouse.y)
        }
    }

    onReleased: {

        interactive = true
        if (singleSelect && !drag_drop_handler.dragging) {
            underMouseItem.exclusiveSelect()
        }
        drag_drop_handler.endDrag(mouse.x, mouse.y)

    }

    onDoubleClicked: {
        if (underMouseItem) {
            underMouseItem.exclusiveSelect()
            // this sets the *viewed* playlist to match the playlist that this
            // media item belongs to so that this media item is shown in the
            // viewport
            viewedMediaSetIndex = inspectedMediaSetIndex
            if (viewedMediaSetIndex.valid) {            
                theSessionData.setPlayheadTo(
                    viewedMediaSetIndex,
                    inspectedMediaSetProperties.values.typeRole == "Timeline")
            }
        }
    }

    property var underMouseItem
    onMouseYChanged: {
        if (pressed) return
        var newUnderMouseItem = mediaList.itemAt(mouseArea.mouseX, mouseArea.mouseY + contentY)
        if (newUnderMouseItem != underMouseItem) {
            if (underMouseItem) underMouseItem.hovered = false
            underMouseItem = newUnderMouseItem
            if (underMouseItem) underMouseItem.hovered = true
        }

        if (underMouseItem) {
            underMouseItem.mouseX = mouseArea.mouseX
            underMouseItem.mouseY = mouseArea.mouseY + contentY - underMouseItem.y
        }
    }
    onContainsMouseChanged: {
        if (containsMouse) hotkey_area.forceActiveFocus()
        else if (underMouseItem) underMouseItem.hovered = false
    }

}
