// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

Rectangle { id: widget

    property color flatColor: XsStyleSheet.panelBgFlatColor
    property color topColor: XsStyleSheet.panelBgGradTopColor
    property color bottomColor: XsStyleSheet.panelBgGradBottomColor

    property real topPosition: 0.0
    property real bottomPosition: 1.0

    gradient: Gradient {
        GradientStop { position: topPosition; color: isFlatTheme ? flatColor : topColor }
        GradientStop { position: bottomPosition; color: isFlatTheme ? flatColor : bottomColor } 
    }

}