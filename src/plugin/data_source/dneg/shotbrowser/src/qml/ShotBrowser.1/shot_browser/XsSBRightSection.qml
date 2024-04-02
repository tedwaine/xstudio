// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0

Item{

    property real visibleWidth: 150

    XsGradientRectangle{
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: panelPadding
        spacing: panelPadding 
        
        XsSBR1Tools{
            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight *2 + parent.spacing*2 + 2
        }
        XsSBR2Views{
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        XsSBR3Actions{
            Layout.fillWidth: true
            Layout.preferredHeight: XsStyleSheet.widgetStdHeight
        }

    }
}