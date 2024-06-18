// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.12

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

Item { 
    
    id: control
    property var tickSpacings: [1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000]
    property real tickSpacing: 1.0
    property real minTickSpacing: 5
    property real scaleFactor: width / (viewportPlayhead.durationFrames-1)
    onScaleFactorChanged: {
        computeTickSpacing()
    }

    property int numBookmarks: {
        // pretty ugly, but only way I can suppress errors before these attr
        // values are intialised
        if (viewportPlayhead.bookmarkedFrames != undefined &&
            viewportPlayhead.bookmarkedFrameColours != undefined &&
            typeof viewportPlayhead.bookmarkedFrameColours.length != "undefined" &&
            typeof viewportPlayhead.bookmarkedFrames.length != "undefined") {
            return Math.min(viewportPlayhead.bookmarkedFrameColours.length, viewportPlayhead.bookmarkedFrames.length)
        }
        return 0;
    }

    XsModelProperty {
        id: __restorePlayAfterScrub
        role: "valueRole"
        index: globalStoreModel.searchRecursive("/ui/qml/restore_play_state_after_scrub", "pathRole")
    }
    property alias restorePlayAfterScrub: __restorePlayAfterScrub.value

    function computeTickSpacing() {
        for (var i in tickSpacings) {
            if (scaleFactor*tickSpacings[i] > minTickSpacing) {
                tickSpacing = scaleFactor*tickSpacings[i]
                return
            }
        }
        tickSpacing = 10
    }

    property int numTicks: width/tickSpacing

    Rectangle {
        opacity: 0.2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: scaleFactor*viewportPlayhead.loopStartFrame
        visible: viewportPlayhead.loopStartFrame != 0 && viewportPlayhead.enableLoopRange != 0
    }

    Rectangle {
        opacity: 0.2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: scaleFactor*(viewportPlayhead.durationFrames-viewportPlayhead.loopEndFrame)
        visible: viewportPlayhead.loopEndFrame < viewportPlayhead.durationFrames != 0 && viewportPlayhead.enableLoopRange != 0
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        Repeater {
            model: control.numTicks
            Rectangle {
                width: 1
                height: isBigTick ? parent.height : parent.height - 4
                color: "white"
                opacity: isBigTick ? 0.4 : 0.25
                x: (tickSpacing * index)
                y: isBigTick ? 0 : 4
                property bool isBigTick: index == Math.round((index/5.0))*5
            }
        }
    }

    property var handleWidth: 7
    property var handleColour: palette.highlight

    // This is the playhead widget - a vertical line with a little upside-down
    // house shape at the top
    Item {

        x: scaleFactor*viewportPlayhead.logicalFrame - width/2
        width: control.handleWidth
        property alias handleColour: control.handleColour

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        layer.enabled: true
        layer.samples: 4

        Shape {

            id: shape

            ShapePath {
                strokeWidth: 1
                strokeColor: handleColour
                fillColor: handleColour
                startX: 0
                startY: 0
                PathLine {x: control.handleWidth; y: 0}
                PathLine {x: control.handleWidth; y: control.handleWidth/2}
                PathLine {x: control.handleWidth/2; y: control.handleWidth/1.5}
                PathLine {x: 0; y: control.handleWidth/2}
            }

            ShapePath {
                strokeWidth: 1.5
                strokeColor: handleColour
                fillColor: handleColour
                startX: control.handleWidth/2
                startY: control.handleWidth/1.5
                PathLine {
                    x: control.handleWidth/2
                    y: control.height
                }
            }
        }
    }

    // Draws the image cache indicator bar(s)
    Item {
        Repeater {
            model: viewportPlayhead.cachedFrames ? viewportPlayhead.cachedFrames.length/2 : []
            Rectangle {
                height: 5
                color: "green"
                opacity: 0.5
                radius: 2.5
                x: scaleFactor*viewportPlayhead.cachedFrames[index*2]
                width: scaleFactor*(viewportPlayhead.cachedFrames[index*2+1])
                y: 10
            }
        }
    }

    Item {
        Repeater {
            model: viewportPlayhead.bookmarkedFrames
            Rectangle {
                height: 5
                color: bmColour ? bmColour : palette.highlight
                property var bmColour: viewportPlayhead.bookmarkedFrameColours[index]
                opacity: 0.5
                radius: 2.5
                // if indicator width is less than diameter (5 pixels) of a dot then
                // we need to adjust the x to ensure the dot is aligned with the frame number
                x: scaleFactor*viewportPlayhead.bookmarkedFrames[index*2] - (w < 5 ? (5-w)/2 : 0.0)
                property var w: scaleFactor*viewportPlayhead.bookmarkedFrames[index*2+1]
                width: Math.max(5, w)
                y: 16
            }
        }
    }
    
    MouseArea {

        id: mouseArea
        cursorShape: Qt.PointingHandCursor
        acceptedButtons: Qt.LeftButton
        anchors.fill: parent
        propagateComposedEvents: true

        property bool preScrubPlaying: false

        onPressed: {
            viewportPlayhead.scrubbingFrames = true
            if (viewportPlayhead.playing) {
                preScrubPlaying = true
                viewportPlayhead.playing = false
            }
        }

        onReleased: {
            viewportPlayhead.scrubbingFrames = false
            if (restorePlayAfterScrub && preScrubPlaying) {
                viewportPlayhead.playing = preScrubPlaying
            }
            preScrubPlaying = false
        }

        onMouseXChanged: {
            if (pressed) {
                viewportPlayhead.logicalFrame = Math.round(mouseX / scaleFactor)
            }
        }

    }
}
