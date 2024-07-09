// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
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
        
        R1Header{
            Layout.fillWidth: true
            Layout.minimumHeight: btnHeight
        }
        Rectangle{
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: XsStyleSheet.panelBgColor
        }
        R1MaskTools{
            Layout.fillWidth: true
            Layout.minimumHeight: (btnHeight * 2)
            Layout.maximumHeight: (btnHeight * 2)
        }
        R2Controls{
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }
}