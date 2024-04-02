// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

Rectangle {

    id: widget

    signal clicked()

    property var text
    property var margin: 10

    width: metrics.width+margin*2
    height: metrics.height+margin
    color: "transparent"
    border.color: palette.highlight
    border.width: mouseArea.containsMouse ? 1 : 0

    XsGradientRectangle{
        id: bgDiv
        anchors.fill: parent

        flatColor: topColor
        topColor: mouseArea.pressed ? palette.highlight: "#33FFFFFF" 
        bottomColor: mouseArea.pressed ? palette.highlight: XsStyleSheet.widgetBgNormalColor
    }

    Text {
        id: textDiv
        text: widget.text
        font.pixelSize: XsStyleSheet.fontSize *1.1
        color: palette.text 
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.centerIn: parent
    }
 
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true 
        onClicked: {
            widget.clicked()
        }
    }

    TextMetrics {
        id: metrics
        font: widget.font
        text: widget.text
    }

}

