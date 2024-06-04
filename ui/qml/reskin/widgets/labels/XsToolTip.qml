// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14

import xStudioReskin 1.0

ToolTip {
    id: widget
    
    property alias textDiv: textDiv
    property alias metricsDiv: metricsDiv

    property color bgColor: palette.text
    property color textColor: palette.base
    property real panelPadding: XsStyleSheet.panelPadding

    delay: 500
    
    font.pixelSize: XsStyleSheet.fontSize
    font.family: XsStyleSheet.fontFamily

    width: metricsDiv.width

    rightPadding: 0
    leftPadding: 0

    TextMetrics {
        id: metricsDiv
        font: textDiv.font
        text: textDiv.text
    }

    contentItem: Text {
        id: textDiv
        text: widget.text
        font: widget.font
        color: textColor
        width: 200
        leftPadding: panelPadding
        rightPadding: panelPadding
        wrapMode: Text.Wrap //WrapAnywhere
    }

    background: Rectangle {
        color: bgColor

        Rectangle {
            id: shadowDiv
            color: "#000000"
            opacity: 0.2
            x: 2
            y: -2
            z: -1
        }
    }

}