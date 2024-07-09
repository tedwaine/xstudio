// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

Rectangle {
	id: control

	property bool isHovered: false
	property bool isEnabled: true
	property bool isSelected: false
	property int start: 0
	property int parentStart: 0
	property int duration: 0
	property real fps: 24.0
	property string name

	color: timelineBackground

	XsElideLabel {
		anchors.fill: parent
		anchors.leftMargin: 5
		anchors.rightMargin: 5
	    horizontalAlignment: Text.AlignHCenter
	    verticalAlignment: Text.AlignVCenter
		text: name
		opacity: 0.4
		elide: Qt.ElideMiddle
		font.pixelSize: 14
		clip: true
		visible: isHovered
		z:1
	}
}
