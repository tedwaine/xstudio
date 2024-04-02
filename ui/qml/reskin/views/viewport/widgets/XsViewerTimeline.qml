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
    property real scaleFactor: width / (playheadDurationFrames-1)
    onScaleFactorChanged: {
        computeTickSpacing()
    }

    XsAttributeValue {
        id: __playheadLogicalFrame
        attributeTitle: "Logical Frame"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadDurationFrames
        attributeTitle: "Duration Frames"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadCachedFrames
        attributeTitle: "Cached Frames"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadBookmarkedFrames
        attributeTitle: "Bookmarked Frames"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadBookmarkedFrameColours
        role: "combo_box_options"
        attributeTitle: "Bookmarked Frames"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadLoopStartFrame
        attributeTitle: "Loop Start Frame"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadLoopEndFrame
        attributeTitle: "Loop End Frame"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadEnableLoopRange
        attributeTitle: "Enable Loop Range"
        model: viewportPlayheadDataModel
    }

    XsAttributeValue {
        id: __playheadPlaying
        attributeTitle: "playing"
        model: viewportPlayheadDataModel
    }

    property alias playheadLogicalFrame: __playheadLogicalFrame.value
    property alias playheadDurationFrames: __playheadDurationFrames.value
    property alias playheadPlaying: __playheadPlaying.value
    property alias playheadCachedFrames: __playheadCachedFrames.value
    property alias playheadBookmarkedFrames: __playheadBookmarkedFrames.value
    property alias playheadBookmarkedFrameColours: __playheadBookmarkedFrameColours.value
    property alias playheadLoopStartFrame: __playheadLoopStartFrame.value
    property alias playheadLoopEndFrame: __playheadLoopEndFrame.value
    property alias playheadEnableLoopRange: __playheadEnableLoopRange.value

    property int numBookmarks: {
        // pretty ugly, but only way I can suppress errors before these attr
        // values are intialised
        if (playheadBookmarkedFrames != undefined &&
            playheadBookmarkedFrameColours != undefined &&
            typeof playheadBookmarkedFrameColours.length != "undefined" &&
            typeof playheadBookmarkedFrames.length != "undefined") {
            return Math.min(playheadBookmarkedFrameColours.length, playheadBookmarkedFrames.length)
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
        width: scaleFactor*playheadLoopStartFrame
        visible: playheadLoopStartFrame != 0 && playheadEnableLoopRange != 0
    }

    Rectangle {
        opacity: 0.2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: scaleFactor*(playheadDurationFrames-playheadLoopEndFrame)
        visible: playheadDurationFrames != 0 && playheadEnableLoopRange != 0
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

        x: scaleFactor*control.playheadLogicalFrame - width/2
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
            model: control.playheadCachedFrames ? control.playheadCachedFrames.length/2 : []
            Rectangle {
                height: 5
                color: "green"
                opacity: 0.5
                radius: 2.5
                x: scaleFactor*control.playheadCachedFrames[index*2]
                width: scaleFactor*(control.playheadCachedFrames[index*2+1])
                y: 10
            }
        }
    }

    Item {
        Repeater {
            model: numBookmarks
            Rectangle {
                height: 5
                color: bmColour ? bmColour : palette.highlight
                property var bmColour: control.playheadBookmarkedFrameColours[index]
                opacity: 0.5
                radius: 2.5
                // if indicator width is less than diameter (5 pixels) of a dot then
                // we need to adjust the x to ensure the dot is aligned with the frame number
                x: scaleFactor*control.playheadBookmarkedFrames[index*2] - (w < 5 ? (5-w)/2 : 0.0)
                property var w: scaleFactor*control.playheadBookmarkedFrames[index*2+1]
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
            if (playheadPlaying) {
                preScrubPlaying = true
                playheadPlaying = false
            }
        }

        onReleased: {
            if (restorePlayAfterScrub && preScrubPlaying) {
                playheadPlaying = preScrubPlaying
            }
            preScrubPlaying = false
        }

        onMouseXChanged: {
            if (pressed) {
                playheadLogicalFrame = Math.round(mouseX / scaleFactor)
            }
        }

    }
}
