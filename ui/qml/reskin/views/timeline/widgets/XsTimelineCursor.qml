// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Shapes 1.12
import xStudioReskin 1.0

Item {
    id: control

    property real thickness: 2
    property color color: playheadActive ? palette.highlight : Qt.darker(palette.base, 1.8)

    property int position: start
    property int start: 0
    property int duration: 0
    property int secondOffset: 0
    property real fractionOffset: 0
    property real fps: 24
    property real tickWidth: (control.width / duration)

    readonly property real cursorX: ((position-start) * tickWidth) - fractionOffset
    property int cursorSize: 20

    property bool playheadActive: currentPlayhead.uuid == timelinePlayhead.uuid

    Rectangle {
        id: line
        width: 2.0
        color: control.color

        x: cursorX
        height: parent.height
    }

    // Note: the qml Shape stuff REALLY slows down drawing of the interface.
    // Avoid!

    // ShapePath {
    //     strokeWidth: control.thickness
    //     fillColor: control.color
    //     strokeColor: control.color

    //     startX: cursorX-(cursorSize/2)
    //     startY: 0

    //     // to bottom right
    //     PathLine {x: cursorX+(cursorSize/2); y: 0}
    //     PathLine {x: cursorX; y: cursorSize}
    //     // PathLine {x: cursorX-(cursorSize/2); y: 0}
    // }
}