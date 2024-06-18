// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

XsGradientRectangle {
	id: control

	// clip:true
	property bool isHovered: false
	property bool isEnabled: true
	property bool isLocked: false
	property bool isSelected: false
	property bool isRolling: false
	property bool showRolling: false

	// property int parentStart: 0
	property int start: 0
	property int duration: 0
	property int availableStart: 0
	property int availableDuration: 1
	// property real fps: 24.0
	property string name
	property color primaryColor: defaultClip
	property var realColor: Qt.tint(timelineBackground, helpers.saturate(helpers.alphate(primaryColor, 0.3), 0.3))
    property bool isMoving: false
    property bool isCopying: false
    property color mediaFlagColour: "transparent"
    // readonly property bool extraDetail: isHovered && height > 60

    opacity: isHovered ? 1.0 : isEnabled ? (isLocked ? 0.6 : 1.0) : 0.3

	border.width: 1
    border.color: isHovered ? palette.highlight : Qt.darker(realColor, 0.8)

	flatColor: topColor

    topColor: isSelected ? Qt.darker(palette.highlight, 2) : realColor
    bottomColor: isSelected ? Qt.darker(palette.highlight, 2) :  Qt.darker(realColor,1.2)

	// XsTickWidget {
	// 	anchors.left: parent.left
	// 	anchors.right: parent.right
	// 	anchors.top: parent.top
	// 	height: Math.min(parent.height/5, 20)
	// 	start: control.start
	// 	duration: control.duration
	// 	fps: control.fps
	// 	endTicks: false
	// }

	// Rectangle {
	// 	color: "transparent"
	// 	z:5
	// 	anchors.fill: parent
	// 	border.width: 1
	// 	border.color: isMoving || isCopying ? "red" : isHovered ? palette.highlight : Qt.lighter(
	// 		Qt.tint(timelineBackground, helpers.saturate(helpers.alphate(mainColor, 0.4), 0.4)),
	// 		1.2)
	// }

	Rectangle {
		anchors.left: parent.left
		anchors.leftMargin: 1.5
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		width: 1.5
		color: mediaFlagColour
		// z: 6
	}

	XsElideLabel {
		anchors.fill: parent
		anchors.leftMargin: 5
		anchors.rightMargin: 5
		anchors.bottomMargin: 5
		elide: Qt.ElideMiddle
		text: name
		opacity: 0.8
		// font.pixelSize: 14
		z:1
		clip: true
	    horizontalAlignment: isRolling ? Text.AlignHCenter : Text.AlignLeft
	    verticalAlignment: isRolling ? Text.AlignVCenter : Text.AlignBottom
	}

	// Label {
	// 	anchors.verticalCenter: parent.verticalCenter
	// 	text: parentStart
	// 	anchors.left: parent.left
	// 	anchors.leftMargin: 10
	// 	visible: isHovered
	// 	z:2
	// }

	// Label {
	// 	anchors.verticalCenter: parent.verticalCenter
	// 	text: parentStart + duration -1
	// 	anchors.right: parent.right
	// 	anchors.rightMargin: 10
	// 	visible: isHovered
	// 	z:2
	// }

	// Label {
	// 	text: duration
	// 	anchors.top: parent.top
	// 	anchors.horizontalCenter: parent.horizontalCenter
	// 	anchors.topMargin: 5
	// 	visible: extraDetail
	// 	z:2
	// }
	// Label {
	// 	anchors.left: parent.left
	// 	anchors.leftMargin: 10
	// 	anchors.topMargin: 5
	// 	text: start
	// 	visible: extraDetail
	// 	z:2
	// }
	// Label {
	// 	anchors.top: parent.top
	// 	anchors.right: parent.right
	// 	anchors.rightMargin: 10
	// 	anchors.topMargin: 5
	// 	text: start + duration - 1
	// 	visible: extraDetail
	// 	z:2
	// }

	// Label {
	// 	text: availableDuration
	// 	anchors.horizontalCenter: parent.horizontalCenter
	// 	anchors.bottom: parent.bottom
	// 	anchors.bottomMargin: 5
	// 	visible: extraDetail
	// 	opacity: 0.5
	// 	z:2
	// }
	// Label {
	// 	anchors.bottom: parent.bottom
	// 	anchors.left: parent.left
	// 	anchors.leftMargin: 10
	// 	anchors.bottomMargin: 5
	// 	text: availableStart
	// 	visible: extraDetail
	// 	opacity: 0.5
	// 	z:2
	// }
	// Label {
	// 	anchors.bottom: parent.bottom
	// 	anchors.right: parent.right
	// 	anchors.rightMargin: 10
	// 	anchors.bottomMargin: 5
	// 	opacity: 0.5
	// 	text: availableStart + availableDuration - 1
	// 	visible: extraDetail
	// 	z:2
	// }


	// position of clip in media
	Rectangle {

		visible: showRolling

		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.topMargin: 1
		anchors.bottomMargin: 1
		anchors.leftMargin: (parent.width / availableDuration) * (start - availableStart)

		color: palette.highlight

		width: ((parent.width / availableDuration) * duration)
		clip: true
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
		clip: true
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
		// color: Qt.darker( control.color, 1.2)
		clip: true
		width: parent.width - ((parent.width / availableDuration) * duration) - ((parent.width / availableDuration) * (start - availableStart))
	}
}
