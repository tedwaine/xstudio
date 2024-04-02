// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

Rectangle { id: widget

    // property bool isFlatTheme: false

    property color flatColor: XsStyleSheet.panelBgFlatColor
    property color topColor: XsStyleSheet.panelBgGradTopColor
    property color bottomColor: XsStyleSheet.panelBgGradBottomColor

    // XsStyleSheet.panelBgColor 

    gradient: Gradient {
        GradientStop { position: 0.0; color: isFlatTheme ? flatColor : topColor }
        GradientStop { position: 1.0; color: isFlatTheme ? flatColor : bottomColor } 
    }

    // Timer {
    //     running: true
    //     repeat: true
    //     interval: 1000
    //     onTriggered: {
    //         isFlatTheme = !isFlatTheme
    //     }
    // }

}