// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

ScrollBar { id: widget

    property color thumbColorPressed: palette.highlight
    property color thumbColorHovered: palette.text
    property color thumbColorNormal: XsStyleSheet.hintColor

    property real thumbWidth: thumb.implicitWidth

    padding: 0
    minimumSize: 0.1

    snapMode: ScrollBar.SnapOnRelease

    contentItem:
    Rectangle { id: thumb
        implicitWidth: 5
        implicitHeight: 5
        radius: width/1.1
        color: widget.pressed ? thumbColorPressed: thumbColorHovered
        opacity: hovered || active ? 0.8 : 0.4
    }
}