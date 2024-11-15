// SPDX-License-Identifier: Apache-2.0

import QtQuick

import QtQuick.Layouts

import xStudio 1.0
import xstudio.qml.bookmarks 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

Item{

    property real visibleWidth: 150

    XsGradientRectangle{
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: panelPadding
        spacing: panelPadding

        R1MaskTools{
            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight
        }
        R2Controls{
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }
}