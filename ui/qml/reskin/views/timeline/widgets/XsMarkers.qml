// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Shapes 1.12
import xStudioReskin 1.0

Rectangle {
	id: control
	property int start: 0
    property int duration: 0
    property real fractionOffset: 0
    property real tickWidth: (control.width / duration)
    property alias model: delegateModel.model
    property int markerWidth: 8

	color: "transparent"

    DelegateModel {
        id: delegateModel
        delegate: Shape {
            id: shape
            property real thickness: 2
            visible: x >=0
            x: (((startRole / rateRole) - start ) * tickWidth) - fractionOffset
            height: control.height
            containsMode: Shape.FillContains
            property var color: helpers.saturate(flagRole, 0.4)

            XsToolTip {
                text: nameRole + "\n\n" + JSON.stringify(commentRole, null, 2)
                visible: hover.hovered && !theTimeline.timelineMarkerMenu.visible
                width: 200
                height: 200
                delay: 750
                x: -width/2
                y: parent.height

            }

            HoverHandler {
                id: hover
                cursorShape: Qt.PointingHandCursor
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: theTimeline.showMarkerMenu(0, 0, delegateModel.modelIndex(index), shape)
            }

            ShapePath {

                strokeWidth: 1
                strokeColor: hover.hovered ? XsStyleSheet.accentColor : shape.color
                fillColor: shape.color

                startX: -markerWidth/2
                startY: 0

                PathLine {x: markerWidth/2; y: 0}
                PathLine {x: 0; y: shape.height}
            }
        }
    }

    Repeater {
        model: delegateModel
    }
}