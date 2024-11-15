// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Layouts

import xStudio 1.0
import "."

ColumnLayout {

    spacing: 0

    XsMediaHeader{

        id: titleBar
        Layout.fillWidth: true
        height: XsStyleSheet.widgetStdHeight
    }

    XsMediaList {

        id: mediaList
        Layout.fillWidth: true
        Layout.fillHeight: true
        itemRowHeight: rowHeight

    }

}
