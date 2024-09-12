// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudio 1.0

Item {
	id: control

	property bool isHovered: false
	property bool isSelected: false
	property string itemFlag: ""
	property string text: ""
	property int trackIndex: 0
	property var setTrackHeaderWidth: function(val) {}
	property string title: "Video Track"

	property bool isEnabled: false
	property bool isLocked: false
	property bool isConformSource: false
	property bool dragTarget: false

	property bool isSizerHovered: false
	property bool isSizerDragging: false

	signal sizerHovered(bool hovered)
	signal sizerDragging(bool dragging)

	signal enabledClicked()
	signal lockedClicked()
	signal conformSourceClicked()
	signal flagSet(string flag, string flag_text)

	XsGradientRectangle {
		id: control_background

		anchors.fill: parent
		anchors.rightMargin: 4

		border.width: 1
        border.color: (isHovered || dragTarget) ? palette.highlight : "transparent"

		flatColor: topColor

        topColor: isSelected ? Qt.darker(palette.highlight, 2) : XsStyleSheet.panelBgGradTopColor
        bottomColor: isSelected ? Qt.darker(palette.highlight, 2) :  XsStyleSheet.panelBgGradBottomColor

		// opacity: isEnabled ? 1.0 : 0.33

		RowLayout {
			spacing: 10
			anchors.fill: parent
			anchors.leftMargin: 10
			anchors.rightMargin: 10
			anchors.topMargin: 5
			anchors.bottomMargin: 5

			Rectangle {
				// Layout.preferredHeight: control.height - 4
				Layout.fillHeight: true
				Layout.preferredWidth: height / 2
				color: itemFlag != "" ? helpers.saturate(itemFlag, 0.4) : control_background.color
				border.width: 2
				border.color: Qt.lighter(color, 1.2)

				MouseArea {
					anchors.fill: parent
					onPressed: showFlagMenu(mouse.x, mouse.y, this, flagSet)
					cursorShape: Qt.PointingHandCursor
				}
			}

		    Label {
		    	Layout.fillHeight: true
		    	Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

		    	horizontalAlignment: Text.AlignLeft
		    	verticalAlignment: Text.AlignVCenter
				text: control.title[0] + trackIndex
		    }

		    XsElideLabel {
		    	Layout.fillHeight: true
		    	Layout.fillWidth: true
		    	Layout.minimumWidth: 30
		    	Layout.alignment: Qt.AlignLeft
				elide: Qt.ElideRight
		    	horizontalAlignment: Text.AlignLeft
		    	verticalAlignment: Text.AlignVCenter
				text: control.text == "" ? control.title : control.text
	        	font.pixelSize: XsStyleSheet.fontSize *1.1

		    }

		    GridLayout {
		    	Layout.fillHeight: true
		    	Layout.alignment: Qt.AlignRight

				XsPrimaryButton{
			    	Layout.fillHeight: true
					Layout.preferredWidth: height
					isActiveViaIndicator: false
    	            isActive: isConformSource
    	            text: "C"
					onClicked: control.conformSourceClicked()
				}

				XsPrimaryButton{
			    	Layout.fillHeight: true
					Layout.preferredWidth: height
					imageDiv.height: height-2
					imageDiv.width: height-2
	                imgSrc: isEnabled ? "qrc:/icons/visibility.svg" : "qrc:/icons/visibility_off.svg"
					isActiveViaIndicator: false
    	            isActive: !isEnabled
					onClicked: control.enabledClicked()
				}


				XsPrimaryButton{
			    	Layout.fillHeight: true
					Layout.preferredWidth: height
					imageDiv.height: height-2
					imageDiv.width: height-2
	                imgSrc: isLocked ? "qrc:/icons/lock.svg" : "qrc:/icons/unlock.svg"
					isActiveViaIndicator: false
    	            isActive: isLocked
					onClicked: control.lockedClicked()
				}
		    }
		}
	}

		// Label {
		// 	anchors.top: parent.top
		// 	anchors.left: parent.left
		// 	anchors.leftMargin: 10
		// 	anchors.topMargin: 5
		// 	text: trimmedStartRole
		// 	visible: extraDetail
		// 	z:4
		// }
		// Label {
		// 	anchors.top: parent.top
		// 	anchors.left: parent.left
		// 	anchors.leftMargin: 40
		// 	anchors.topMargin: 5
		// 	text: trimmedDurationRole
		// 	visible: extraDetail
		// 	z:4
		// }
		// Label {
		// 	anchors.top: parent.top
		// 	anchors.left: parent.left
		// 	anchors.leftMargin: 70
		// 	anchors.topMargin: 5
		// 	text: trimmedDurationRole ? trimmedStartRole + trimmedDurationRole - 1 : 0
		// 	visible: extraDetail
		// 	z:4
		// }


	Rectangle {
		width: 4
		height: parent.height

		anchors.right: parent.right
		anchors.top: parent.top
		// color: timelineBackground

        color: isSizerDragging ? palette.highlight : isSizerHovered  ? XsStyleSheet.secondaryTextColor : timelineBackground

		MouseArea {
			id: ma
			anchors.fill: parent
            hoverEnabled: true
			acceptedButtons: Qt.LeftButton
	        preventStealing: true
			cursorShape: Qt.SizeHorCursor

			onContainsMouseChanged: sizerHovered(containsMouse)

			onPressedChanged: sizerDragging(pressed)

			onPositionChanged: {
				if(pressed) {
					let ppos = mapToItem(control, mouse.x, 0)
					setTrackHeaderWidth(ppos.x + 4)
				}
			}
		}
	}

	XsDragDropHandler {

        id: drag_drop_handler
        onDragEntered: {
            if (source == "MediaList") {
                dragTarget = true
            }
        }

        onDragExited: {
            dragTarget = false
        }

		onDropped: {

            if (!dragTarget) return
            dragTarget = false
			if (source == "MediaList" && typeof data == "object" && data.length) {
				// drops from MediaList is a QModelIndexList - the seletcted media from
				// the MediaList that is being dragged over

				// root playlist:
				theSessionData.dump(data[0].parent.parent)
				theSessionData.dump(modelIndex())

				var rc = theSessionData.rowCount(modelIndex());
				for (var c = 0; c < data.length; ++c) {
					var mediaName = theSessionData.get(data[c], "pathRole")
					theSessionData.insertTimelineClip(rc+c, modelIndex(), data[c], mediaName)
				}

			}

        }
	}

}

