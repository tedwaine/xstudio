// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

Image {
    id: widget

    property bool isIcon: true //#TODO: to make XsIcon Widget
    property color imgOverlayColor: isIcon? palette.text : "transparent"
    
    source: ""
    // width: parent.height-4
    // height: parent.height-4
    // topPadding: 2
    // bottomPadding: 2
    // leftPadding: 8
    // rightPadding: 8

    sourceSize.height: height
    sourceSize.width: width

    horizontalAlignment: Image.AlignHCenter
    verticalAlignment: Image.AlignVCenter
    
    smooth: true
    antialiasing: true
    asynchronous: true

    layer {
        enabled: true
        effect: ColorOverlay { color: imgOverlayColor }
    }
}