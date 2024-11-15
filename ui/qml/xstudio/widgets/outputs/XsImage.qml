// SPDX-License-Identifier: Apache-2.0
import QtQuick

import Qt5Compat.GraphicalEffects


import xStudio 1.0

Image {
    id: widget

    property bool isIcon: true //#TODO: to make XsIcon Widget
    property color imgOverlayColor: isIcon? palette.text : "transparent"

    // source: ""

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