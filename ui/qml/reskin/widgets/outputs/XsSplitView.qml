// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

SplitView {

    property real thumbWidth: XsStyleSheet.panelPadding/2
    property color colorNormal: XsStyleSheet.primaryTextColor
    property color colorActive: XsStyleSheet.accentColor

    focus: false

    orientation: Qt.Horizontal
    handle: orientation === Qt.Horizontal? splitHandleHorizontal : splitHandleVertical

    property Component splitHandleHorizontal:
    Rectangle {
        implicitWidth: thumbWidth
        implicitHeight: thumbWidth
        color: XsStyleSheet.panelBgColor
 
        Rectangle{
            width: parent.width/2
            height: parent.height
            color: parent.SplitHandle.pressed ? colorActive : parent.SplitHandle.hovered ? colorNormal : "transparent"
            anchors.centerIn: parent
        }
        // Image {
        //     visible: SplitHandle.hovered
        //     anchors.centerIn: parent
        //     source: "qrc:/icons/more_horiz.svg"
        //     height: panelPadding
        //     width: panelPadding*3
        //     rotation: 90
        //     smooth: true
        //     layer {
        //         enabled: true
        //         effect:
        //         ColorOverlay {
        //             color: SplitHandle.pressed? colorActive : colorNormal
        //         }
        //     }
        // }
    }

    property Component splitHandleVertical:
    Rectangle {
        implicitWidth: XsStyleSheet.panelPadding
        implicitHeight: XsStyleSheet.panelPadding
        color: XsStyleSheet.panelBgColor
        
        Rectangle{
            width: parent.width
            height: XsStyleSheet.panelPadding/2
            color: parent.SplitHandle.pressed ? colorActive : parent.SplitHandle.hovered ? colorNormal : "transparent"
            anchors.centerIn: parent
        }
    }

}