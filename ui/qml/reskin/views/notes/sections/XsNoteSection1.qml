// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import xstudio.qml.helpers 1.0

Rectangle{ id: sec1
    color: "transparent"

    property bool isHovered: thumbMArea.containsMouse    

    MouseArea { id: thumbMArea
        anchors.fill: parent
        propagateComposedEvents: true
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onClicked: {

            // user has clicked on the 'Go to frame'.            
            if (theSessionData.get(viewedMediaSetIndex, "typeRole") == "Timeline") {

                var media_idx = theSessionData.searchRecursive(ownerRole, "actorUuidRole", viewedMediaSetIndex)
                // if we are seeking to a note within a timeline, need some different
                // logic here.
                if (media_idx.valid) {
                    let c = theSessionData.getTimelineVisibleClipIndexes(
                        viewedMediaSetIndex,
                        media_idx,
                        frameFromTimecodeRole
                        )

                    if(c.length) {
                        currentPlayhead.logicalFrame = theSessionData.getTimelineFrameFromClip(c[0], frameFromTimecodeRole)                        
                        // this is a signal we emit - XsTimeline has a connection to
                        // the signal allowing us to control timeline selection
                        theSessionData.makeTimelineSelection(viewedMediaSetIndex, [c[0]]);
                        return
                    }
                }

            }

            theSessionData.putMediaOnScreen(ownerRole)

            // wait 100ms before setting playhead frame, because it might take
            // a moment for the 'currentPlayheadData' to connect to a playhead
            // if the playhead has changed (if we're jumping to a new playlist)
            callbackTimer.setTimeout(function(frame) { return function() {
                currentPlayhead.logicalFrame = frame
                }}(startFrameRole), 100);

                list.currentIndex = index
           
        }
    }


    Item{
        anchors.fill: parent
                
        XsImagePainter { id: thumb
            image: thumbnailRole
            width: parent.width
            height: width / (16/9)
            
        }

        Rectangle{visible: isActive; anchors.fill: parent; color: "transparent"; border.width: borderWidth*2; border.color: highlightColor }

        XsText{
            visible: sec1.isHovered
            text: "Go To Frame"
            anchors.centerIn: thumb
            style: Text.Outline
            font.pixelSize: XsStyleSheet.fontSize + 4
            color: highlightColor
        }
        XsText{
            width: parent.width
            height: itemHeight
            text: "Frame " + startFrameRole
            anchors.bottom: parent.bottom
        }
    }
        

}