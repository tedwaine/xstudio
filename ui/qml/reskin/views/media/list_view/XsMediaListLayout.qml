// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
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
        model: mediaListModelData
        Layout.fillWidth: true
        Layout.fillHeight: true
        property var columns_model_index: titleBar.columns_model_index
        itemRowHeight: rowHeight

    }

}
