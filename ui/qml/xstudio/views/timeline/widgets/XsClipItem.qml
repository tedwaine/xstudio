// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudio 1.0

XsGradientRectangle {
	id: control

	property bool isHovered: false
	property bool isEnabled: true
	property bool isLocked: false
	property bool isSelected: false
	property bool isDragging: false
	property bool isBroken: false

	property bool showRolling: false
	property bool showDragLeft: false
	property bool showDragRight: false
	property bool showDragLeftLeft: false
	property bool showDragRightRight: false
	property bool showDragMiddle: false

	property int start: 0
	property int duration: 0
	property int availableStart: 0
	property int availableDuration: 1
	property string name
	property color primaryColor: defaultClip
	property var realColor: Qt.tint(timelineBackground, helpers.saturate(helpers.alphate(primaryColor, 0.3), 0.3))
    property bool isMoving: false
    property bool isCopying: false
    property color mediaFlagColour: "transparent"

	signal draggingStarted(mode: string)
	signal dragging(mode: string, x: real)
	signal draggingStopped(mode: string)
	signal doubleTapped(mode: string)

    opacity: isHovered ? 1.0 : isEnabled ? (isLocked ? 0.6 : 1.0) : 0.3

	border.width: 1
    border.color: isHovered ? palette.highlight : (isBroken && isEnabled ? "Red" : Qt.darker(realColor, 0.8))

	flatColor: topColor

    topColor: isSelected ? Qt.darker(palette.highlight, 2) : realColor
    bottomColor: isSelected ? Qt.darker(palette.highlight, 2) :  Qt.darker(realColor,1.2)

	XsGradientRectangle {
		anchors.left: parent.left
		anchors.leftMargin: 1
		anchors.topMargin: 1
		anchors.bottomMargin: 1
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		width: 10
		orientation: Gradient.Horizontal
		topColor: mediaFlagColour
		bottomColor: control.bottomColor
		flatTheme: false
		visible: mediaFlagColour != "transparent" && mediaFlagColour != "#00000000"
		opacity: showDragMiddle ? 0.2 : 0.6
	}

	XsElideLabel {
		anchors.fill: parent
		anchors.leftMargin: 11
		anchors.rightMargin: 5
		anchors.bottomMargin: 5
		elide: Qt.ElideMiddle
		text: name
		z:2
		color: palette.text
	    horizontalAlignment: isDragging ? Text.AlignHCenter : Text.AlignLeft
	    verticalAlignment: isDragging ? Text.AlignVCenter : Text.AlignBottom
		opacity: 0.8
	}

	readonly property int dragWidth: 4

	Rectangle {
		z: 0
		radius: 2
		visible: showDragMiddle
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.topMargin: 2
		anchors.bottomMargin: 2
		anchors.horizontalCenter: parent.horizontalCenter
		width: control.width
		color: "transparent"
		// opacity: hoverMiddleHandler.hovered ? 1.0 : 0.5

        HoverHandler {
        	id: hoverMiddleHandler
            cursorShape: Qt.PointingHandCursor
        }

        TapHandler {
        	onDoubleTapped: control.doubleTapped("middle")
        }

        DragHandler {
            cursorShape: Qt.PointingHandCursor
            yAxis.enabled: false

            dragThreshold: 1
            xAxis.minimum: (control.width/2) - dragWidth
            xAxis.maximum: (control.width/2) + dragWidth

            onTranslationChanged: dragging("middle", translation.x)
            onActiveChanged: {
            	if(active) {
            		draggingStarted("middle")
            		parent.anchors.horizontalCenter = undefined
            	} else {
            		draggingStopped("middle")
	            	parent.anchors.horizontalCenter = parent.parent.horizontalCenter
	            }
            }
        }
	}

	Rectangle {
		radius: 2
		visible: showDragLeft
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.topMargin: 6
		anchors.bottomMargin: 6
		anchors.leftMargin: dragWidth * 2
		width: dragWidth
		color: palette.highlight
		opacity: hoveredLeftHandler.hovered ? 1.0 : 0.5

        HoverHandler {
        	id: hoveredLeftHandler
            cursorShape: Qt.PointingHandCursor
        }

        // modifies the activeStart / activeDuration frame, bounded by duration>0 and availableStartFrame
        // also bounded by precedding item being a gap.

        DragHandler {
            cursorShape: Qt.PointingHandCursor
            yAxis.enabled: false
            dragThreshold: 1
            onTranslationChanged: dragging("left", translation.x)
            xAxis.minimum: (dragWidth * 2) - 1
            xAxis.maximum: (dragWidth * 2) + 1
            onActiveChanged: {
            	if(active) {
            		draggingStarted("left")
            		parent.anchors.left = undefined
            	} else {
            		draggingStopped("left")
	            	parent.anchors.left = parent.parent.left
	            }
            }
        }
	}

	Rectangle {
		radius: 2
		visible: showDragRight
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.topMargin: 6
		anchors.bottomMargin: 6
		anchors.rightMargin: dragWidth * 2
		width: dragWidth
		color: palette.highlight
		opacity: hoveredRightHandler.hovered ? 1.0 : 0.5

        HoverHandler {
        	id: hoveredRightHandler
            cursorShape: Qt.PointingHandCursor
        }

        DragHandler {
            cursorShape: Qt.PointingHandCursor
            yAxis.enabled: false
            dragThreshold: 1

            xAxis.minimum: control.width - (dragWidth * 3) - 1
            xAxis.maximum: control.width - (dragWidth * 3) + 1

            onTranslationChanged: dragging("right", translation.x)
            onActiveChanged: {
            	if(active) {
            		draggingStarted("right")
            		parent.anchors.right = undefined
            	} else {
            		draggingStopped("right")
	            	parent.anchors.right = parent.parent.right
	            }
            }
        }
   	}

	Rectangle {
		radius: 2
		visible: showDragLeftLeft
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.topMargin: 2
		anchors.bottomMargin: 2
		width: dragWidth
		color: palette.highlight
		opacity: dragLeftLeftHandler.hovered ? 1.0 : 0.5

        HoverHandler {
        	id: dragLeftLeftHandler
            cursorShape: Qt.PointingHandCursor
        }

        DragHandler {
            cursorShape: Qt.PointingHandCursor
            yAxis.enabled: false
            dragThreshold: 1
            xAxis.minimum: - 1
            xAxis.maximum: 1
            onTranslationChanged: dragging("leftleft", translation.x)
            onActiveChanged: {
            	if(active) {
            		draggingStarted("leftleft")
            		parent.anchors.left = undefined
            	} else {
            		draggingStopped("leftleft")
	            	parent.anchors.left = parent.parent.left
	            }
            }
        }
	}

	Rectangle {
		radius: 2
		visible: showDragRightRight
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.topMargin: 2
		anchors.bottomMargin: 2
		width: dragWidth
		color: palette.highlight
		opacity: dragRightRightHandler.hovered ? 1.0 : 0.5

        HoverHandler {
        	id: dragRightRightHandler
            cursorShape: Qt.PointingHandCursor
        }
        DragHandler {
            cursorShape: Qt.PointingHandCursor
            yAxis.enabled: false
            dragThreshold: 1
            xAxis.minimum: control.width - dragWidth
            xAxis.maximum: control.width
            onTranslationChanged: dragging("rightright", translation.x)
            onActiveChanged: {
            	if(active) {
            		draggingStarted("rightright")
            		parent.anchors.right = undefined
            	} else {
            		draggingStopped("rightright")
	            	parent.anchors.right = parent.parent.right
	            }
            }
        }
	}

	Rectangle {
		radius: 2
		visible: showRolling

		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.topMargin: 1
		anchors.bottomMargin: 1
		anchors.leftMargin: (parent.width / availableDuration) * (start - availableStart)

		width: ((parent.width / availableDuration) * duration)
		color: palette.highlight
		opacity: rollHandler.hovered ? 1.0 : 0.5

        HoverHandler {
        	id: rollHandler
            cursorShape: Qt.PointingHandCursor
        }
        DragHandler {
            cursorShape: Qt.PointingHandCursor
            yAxis.enabled: false
            dragThreshold: 1
            xAxis.minimum: 0
            xAxis.maximum: control.width - parent.width
            onTranslationChanged: dragging("roll", translation.x)
            onActiveChanged: {
            	if(active) {
            		draggingStarted("roll")
            		parent.anchors.left = undefined
            	} else {
            		draggingStopped("roll")
	            	parent.anchors.left = parent.parent.left
	            }
            }
        }
	}
}
