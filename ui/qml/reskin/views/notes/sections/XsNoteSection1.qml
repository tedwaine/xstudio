// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import xstudio.qml.helpers 1.0

Rectangle{
    color: "transparent"

    property bool isHovered: thumbMArea.containsMouse    

    MouseArea { id: thumbMArea
        anchors.fill: parent
        propagateComposedEvents: true
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onClicked: { //#TODO: for test

            theSessionData.putMediaOnScreen(ownerRole)

            // wait 100ms before setting playhead frame, because it might take
            // a moment for the 'currentPlayheadData' to connect to a playhead
            // if the playhead has changed (if we're jumping to a new playlist)
            callbackTimer.setTimeout(function(frame) { return function() {
                var idx = currentPlayheadData.searchRecursive("Logical Frame", "title")
                currentPlayheadData.set(idx, frame, "value")
                }}(startFrameRole), 100);
           
        }
        onDoubleClicked: isTestMode = !isTestMode
    }


    Item{
        anchors.fill: parent
                
        XsImagePainter {
            image: thumbnailRole
            width: parent.width
            height: width / (16/9)
            
        }

        Rectangle{visible: isActive; anchors.fill: parent; color: "transparent"; border.width: borderWidth*2; border.color: highlightColor }

        XsText{
            visible: isHovered
            text: "Go To Frame"
            anchors.centerIn: parent
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