// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

XsGradientRectangle {
	id: control

	property bool isHovered: false
	property bool isEnabled: true
	property bool isLocked: false
	property bool isSelected: false
	property bool isRolling: false
	property bool showRolling: false
	property bool isBroken: false

	property bool dragLeft: false
	property bool dragRight: false
	property bool dragLeftLeft: false
	property bool dragRightRight: false
	property bool dragMiddle: false

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
		opacity: dragMiddle ? 0.2 : 0.6
	}

	XsElideLabel {
		anchors.fill: parent
		anchors.leftMargin: 11
		anchors.rightMargin: 5
		anchors.bottomMargin: 5
		elide: Qt.ElideMiddle
		text: name
		// font.pixelSize: 14
		z:1
		// clip: true
	    horizontalAlignment: isRolling ? Text.AlignHCenter : Text.AlignLeft
	    verticalAlignment: isRolling ? Text.AlignVCenter : Text.AlignBottom
		opacity: dragMiddle ? 0.2 : 0.8
	}

	readonly property int dragWidth: 6

	Rectangle {
		radius: 2
		visible: dragMiddle
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.topMargin: 2
		anchors.bottomMargin: 2
		anchors.horizontalCenter: parent.horizontalCenter
		width: dragWidth * 2
		color: palette.highlight
		opacity: hoverMiddleHandler.hovered ? 1.0 : 0.5

        HoverHandler {
        	id: hoverMiddleHandler
            cursorShape: Qt.PointingHandCursor
        }

        DragHandler {
            cursorShape: Qt.PointingHandCursor
            yAxis.enabled: false

            dragThreshold: 1
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
		visible: dragLeft
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
            xAxis.minimum: 0
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
		visible: dragRight
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
		visible: dragLeftLeft
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
            xAxis.minimum: 0
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
		visible: dragRightRight
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


	// position of clip in media
	Rectangle {
		radius: 2

		visible: showRolling

		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.topMargin: 1
		anchors.bottomMargin: 1
		anchors.leftMargin: (parent.width / availableDuration) * (start - availableStart)

		color: palette.highlight

		width: ((parent.width / availableDuration) * duration)
	}

	// position of clip in media
	Rectangle {

		visible: showRolling

		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.topMargin: 1
		anchors.bottomMargin: 1
		anchors.leftMargin: 1

		color: "black"
		opacity: 0.1

		width: (parent.width / availableDuration) * (start - availableStart)
	}

	Rectangle {
		visible: showRolling

		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.topMargin: 1
		anchors.bottomMargin: 1
		anchors.rightMargin: 1

		color: "black"
		opacity: 0.1
		width: parent.width - ((parent.width / availableDuration) * duration) - ((parent.width / availableDuration) * (start - availableStart))
	}
}
