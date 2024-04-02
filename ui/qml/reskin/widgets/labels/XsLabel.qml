// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

Label{
    id: widget

    property color textColorNormal: palette.text
    property var textElide: widget.elide
    property real textWidth: metrics.width
    property alias toolTip: toolTip
    property alias tooltipText: toolTip.text
    property alias tooltipVisibility: toolTip.visible
    property real toolTipWidth: widget.width+5 //150
    property bool shadow: false

    text: ""
    color: textColorNormal
    elide: Text.ElideRight
    wrapMode: Text.WordWrap

    font.pixelSize: XsStyleSheet.fontSize
    font.family: XsStyleSheet.fontFamily

    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter

    // width: parent.width
    // height: parent.height
    // topPadding: 2
    // bottomPadding: 2
    // leftPadding: 20
    // rightPadding: 20

    DropShadow {
        anchors.fill: widget
        horizontalOffset: 0
        verticalOffset: 0
        radius: 1.0
        samples: 4
        color: "#ff000000"
        source: widget
        visible: shadow
        z: -1
    }

    TextMetrics {
        id: metrics
        font: widget.font
        text: widget.text
    }

    // MouseArea{
    //     id: mArea
    //     anchors.fill: parent
    //     hoverEnabled: true
    //     propagateComposedEvents: true
    // }

    XsToolTip{
        id: toolTip
        text: widget.text
        visible: false //mArea.containsMouse && parent.truncated
        width: metrics.width == 0? 0 : toolTipWidth
        x: 0 //#TODO: flex/pointer
    }
}