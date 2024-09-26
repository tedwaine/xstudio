// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtGraphicalEffects 1.0
import QuickFuture 1.0
import QuickPromise 1.0

import xStudio 1.0
import xstudio.qml.helpers 1.0

RowLayout {
	id: control
	spacing: 0

	property var config: ListView.view || control.parent

	width: (durationFrame + adjustPreceedingGap + adjustAnteceedingGap) * config.scaleX
	height: config.scaleY * config.itemHeight

	property bool showDragLeft: false
	property bool showDragRight: false
	property bool showDragMiddle: false
	property bool showDragLeftLeft: false
	property bool showDragRightRight: false
	property bool showRolling: false

	property bool isAdjustPreceeding: false
	property bool isAdjustAnteceeding: false

	property int adjustPreceedingGap: 0
	property int adjustAnteceedingGap: 0

	property bool isBothHovered: false

	property int adjustDuration: "adjust_duration" in userDataRole ? userDataRole.adjust_duration : 0
	property bool isAdjustingDuration: "is_adjusting_duration" in userDataRole ? userDataRole.is_adjusting_duration : false
	property int adjustStart: "adjust_start" in userDataRole ? userDataRole.adjust_start : 0
	property bool isAdjustingStart: "is_adjusting_start" in userDataRole ? userDataRole.is_adjusting_start : false

	property int startFrame: isAdjustingStart ? trimmedStartRole + adjustStart : trimmedStartRole
	property int durationFrame: isAdjustingDuration ? trimmedDurationRole + adjustDuration : trimmedDurationRole

	property int currentDurationFrame: trimmedDurationRole
	property real fps: rateFPSRole

	property var timelineSelection: config.timelineSelection
    property var timelineItem: config.timelineItem
    property var itemTypeRole: typeRole
    property var hoveredItem: config.hoveredItem
    property var scaleX: config.scaleX
    property var parentLV: config

    property bool isParentLocked: config.isParentLocked
    property bool isParentEnabled: config.isParentEnabled

    property var draggingStarted: config.draggingStarted
    property var dragging: config.dragging
    property var draggingStopped: config.draggingStopped
    property var doubleTapped: config.doubleTapped


    property string itemFlag: flagColourRole != "" ? flagColourRole : config.itemFlag

    property bool hasMedia: !trimmedDurationRole || clipMediaUuidRole == "{00000000-0000-0000-0000-000000000000}" || mediaIndex.valid
    property alias mediaIndex: mediaStatus.index

    property alias isSelected: clip.isSelected
    property var isLocked: lockedRole

	property var mediaUuid: clipMediaUuidRole

	onMediaUuidChanged: updateMediaIndex()

	onHoveredItemChanged: isBothHovered = false

	XsModelProperty {
		id: mediaStatus
		role: "mediaStatusRole"
		onIndexChanged: {
			if(!index.valid && clipMediaUuidRole && clipMediaUuidRole != "{00000000-0000-0000-0000-000000000000}")
				updateMediaIndex()
		}
	}

	function modelIndex() {
		return helpers.makePersistent(DelegateModel.model.modelIndex(index))
	}

    Timer {
        id: updateTimer
        interval: 500
        running: false
        repeat: false
        onTriggered: {
        	updateMediaIndex()
        }
    }

    function updateMediaIndex(retry=true) {
    	let m = DelegateModel.model.srcModel
    	let tindex = m.getTimelineIndex(DelegateModel.model.modelIndex(index))
    	let mlist = m.index(0, 0, tindex)
    	let result = m.search(clipMediaUuidRole, "actorUuidRole", mlist)

    	if(retry && !result.valid && clipMediaUuidRole && clipMediaUuidRole != "{00000000-0000-0000-0000-000000000000}" && !updateTimer.running) {
    		updateTimer.start()
    	} else {
    		result = helpers.makePersistent(result)
    		if(mediaStatus.index != result)
    			mediaStatus.index = result
    		if(mediaFlag.index != result)
    			mediaFlag.index = result
    	}
    }

	function adjust(offset) {
		let doffset = offset
		let tmp = userDataRole

		if(isAdjustingStart) {
			tmp.adjust_start = offset
			doffset = -doffset
		}

		if(isAdjustingDuration) {
			tmp.adjust_duration = doffset
		}

		if(isAdjustingDuration || isAdjustingStart) {
			userDataRole = tmp
		}
	}

	function checkAdjust(offset, lock_duration=false, lock_end=false) {
		let doffset = offset

		if(isAdjustingStart) {
			let tmp = Math.min(
				availableStartRole+availableDurationRole-1,
				Math.max(trimmedStartRole + offset, availableStartRole)
			)

			if(lock_end && tmp > trimmedStartRole+trimmedDurationRole) {
				tmp = trimmedStartRole+trimmedDurationRole-1
			}

			if(trimmedStartRole != tmp-offset) {
				return checkAdjust(tmp-trimmedStartRole)
			}

			// if adjusting duration as well
			doffset = -doffset
		}

		if(isAdjustingDuration && lock_duration) {
			let tmp = Math.max(
				1,
				Math.min(trimmedDurationRole + doffset, availableDurationRole - (startFrame-availableStartRole) )
			)

			if(trimmedDurationRole != tmp-doffset) {
				if(isAdjustingStart)
					return checkAdjust(-(tmp-trimmedDurationRole))
				else
					return checkAdjust(tmp-trimmedDurationRole)
			}
		}

		return offset
	}


    function updateStart(change) {
		let tmp = userDataRole

		tmp.adjust_start = Math.floor(Math.min(
			Math.max(trimmedStartRole + change, availableStartRole),
			availableStartRole + availableDurationRole - trimmedDurationRole
		) - trimmedStartRole)

		userDataRole = tmp
    }

	XsGapItem {
		visible: adjustPreceedingGap != 0
		Layout.preferredWidth: adjustPreceedingGap * scaleX
		Layout.fillHeight: true
		start: 0
		duration: adjustPreceedingGap
	}

	XsClipItem {
		id: clip

		Layout.preferredWidth: durationFrame * scaleX
		Layout.fillHeight: true

		isHovered: hoveredItem == control || isAdjustingStart || isAdjustingDuration || isBothHovered
		start: startFrame
		duration: durationFrame
		isLocked: (isParentLocked || lockedRole)
		isEnabled: isParentEnabled && enabledRole
		isBroken: !hasMedia || (mediaStatus.value != undefined && mediaStatus.value != "Online") || !activeRangeValidRole

		showRolling: isSelected && isHovered && control.showRolling && !isParentLocked && !lockedRole
		showDragLeft: isSelected && isHovered && control.showDragLeft && !isParentLocked && !lockedRole
		showDragRight: isSelected && isHovered && control.showDragRight && !isParentLocked && !lockedRole
		showDragMiddle: isSelected && isHovered && control.showDragMiddle && !isParentLocked && !lockedRole
		showDragLeftLeft: isSelected && isHovered && control.showDragLeftLeft && !isParentLocked && !lockedRole
		showDragRightRight: isSelected && isHovered && control.showDragRightRight && !isParentLocked && !lockedRole

		// onIsBrokenChanged: {
		// 	if(isBroken) {
		// 		console.log(nameRole, hasMedia, mediaStatus.value, activeRangeValidRole)
		// 		console.log("avail", availableStartRole, availableDurationRole, availableStartRole + availableDurationRole-1)
		// 		console.log("active", activeStartRole, activeDurationRole, activeStartRole + activeDurationRole-1)
		// 		console.log("trimmed", trimmedStartRole, trimmedDurationRole, trimmedStartRole + trimmedDurationRole-1)
		// 	}
		// }
		// fps: control.fps
		name: !isDragging ? nameRole : isAdjustingStart ? (adjustStart > 0 ? "+" + adjustStart : adjustStart) : (adjustDuration > 0 ? "+" + adjustDuration : adjustDuration)
		availableStart: availableStartRole
		availableDuration: availableDurationRole
		primaryColor: itemFlag != "" ?  itemFlag : defaultClip
        mediaFlagColour: mediaFlag.value == undefined || mediaFlag.value == "" ? "transparent" : mediaFlag.value

	    XsModelProperty {
	        id: mediaFlag
	        role: "flagColourRole"
			onIndexChanged: {
				if(!index.valid && clipMediaUuidRole && clipMediaUuidRole != "{00000000-0000-0000-0000-000000000000}")
					updateMediaIndex()
			}
	    }

	    Component.onCompleted: {
	    	updateMediaIndex()
	    }

	    onDraggingStarted: {
	    	control.draggingStarted(modelIndex(), control, mode)
	    	isDragging = true
	    }
		onDragging: control.dragging(modelIndex(), control, mode, x / scaleX)
		onDoubleTapped: control.doubleTapped(control, mode)
		onDraggingStopped: {
			control.draggingStopped(modelIndex(), control, mode)
	    	isDragging = false
		}

	    Connections {
	    	target: dragContainer.dragged_items
	    	function onSelectionChanged() {
	    		if(dragContainer.dragged_items.selectedIndexes.length) {
	    			if(dragContainer.dragged_items.isSelected(modelIndex())) {
	    				if(dragContainer.Drag.supportedActions == Qt.CopyAction)
	    					clip.isCopying = true
	    				else
	    					clip.isMoving = true
	    			}
	    		} else {
	    			clip.isMoving = false
	    			clip.isCopying = false
	    		}
	    	}
	    }

		Connections {
			target: control.timelineSelection
			function onSelectionChanged(selected, deselected) {
				if(clip.isSelected && helpers.itemSelectionContains(deselected, modelIndex()))
					clip.isSelected = false
				else if(!clip.isSelected && helpers.itemSelectionContains(selected, modelIndex()))
					clip.isSelected = true
			}
		}
	}

	XsGapItem {
		visible: adjustAnteceedingGap != 0
		Layout.preferredWidth: adjustAnteceedingGap * scaleX
		Layout.fillHeight: true
		start: 0
		duration: adjustAnteceedingGap
	}
}
