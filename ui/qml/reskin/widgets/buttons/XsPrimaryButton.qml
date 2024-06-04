// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

Button {
    id: widget

    property alias imgSrc: imageDiv.source
    property bool isActive: false
    property bool isUnClickable: false
    property bool isActiveViaIndicator: true
    property bool isActiveIndicatorAtLeft: false
    property bool isToolTipEnabled: true

    property alias textDiv: textDiv
    property alias imageDiv: imageDiv
    property alias bgDiv: bgDiv
    property alias activeIndicator: activeIndicator
    property real rotaionAnimDuration: 150

    property color imgOverlayColor: palette.text
    property color textColor: palette.text
    property color bgColorPressed: palette.highlight
    property color bgColorNormal: XsStyleSheet.widgetBgNormalColor
    property color forcedBgColorNormal: bgColorNormal
    property color borderColorHovered: bgColorPressed
    property color borderColorNormal: "transparent"
    property real borderWidth: 1

    focusPolicy: Qt.NoFocus
    hoverEnabled: true

    font.pixelSize: XsStyleSheet.fontSize *1.1
    font.weight: Font.Medium

    implicitWidth: textDiv.textWidth + 10

    contentItem:
    Item{
        anchors.fill: parent
        opacity: enabled || isUnClickable? 1.0 : 0.33
        clip: true

        XsImage {
            id: imageDiv
            source: ""
            sourceSize.height: 20
            sourceSize.width: 20
            anchors.centerIn: parent
            imgOverlayColor: palette.text

            Behavior on rotation {NumberAnimation{duration: rotaionAnimDuration }}
        }

        XsText {
            id: textDiv
            visible: imgSrc==""
            text: widget.text
            font: widget.font
            color: textColor
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: parent.height
        }
        XsToolTip{
            text: widget.text
            visible: textDiv.visible? widget.hovered && textDiv.truncated : widget.hovered && widget.text!=""
            width: metricsDiv.width == 0? 0 : textDiv.textWidth +15
            // height: widget.height
            x: widget.width //#TODO: flex/pointer
            y: widget.height
        }
    }

    background:
    XsGradientRectangle {
        id: bgDiv
        implicitWidth: 100
        implicitHeight: 40
        border.color: widget.down || widget.hovered ? borderColorHovered: borderColorNormal
        border.width: borderWidth

        opacity: enabled? 1.0 : 0.33

        flatColor: topColor
        topColor: widget.down || (isActive && !isActiveViaIndicator)? bgColorPressed: forcedBgColorNormal==bgColorNormal?"#33FFFFFF":forcedBgColorNormal
        bottomColor: widget.down || (isActive && !isActiveViaIndicator)? bgColorPressed: forcedBgColorNormal
    

        Rectangle {
            id: bgFocusDiv
            implicitWidth: parent.width+borderWidth
            implicitHeight: parent.height+borderWidth
            visible: widget.activeFocus
            color: "transparent"
            opacity: 0.33
            border.color: borderColorHovered
            border.width: borderWidth
            anchors.centerIn: parent
        }
        Rectangle{ id: activeIndicator
            anchors.bottom: parent.bottom
            width: isActiveIndicatorAtLeft? borderWidth*3 : parent.width;
            height: isActiveIndicatorAtLeft? parent.height : borderWidth*3
            color: isActiveViaIndicator && isActive? bgColorPressed : "transparent"
        }
    }

    /*onPressed: focus = true
    onReleased: focus = false*/

}

