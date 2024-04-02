// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0

Item{

    XsGradientRectangle{
        anchors.fill: parent
    }

    property real visibleWidth: 450

    ColumnLayout { id: leftView
        anchors.fill: parent
        anchors.margins: panelPadding
        spacing: panelPadding

        XsSBL1Tools{
            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight
        }
        XsSBL2Views{ id: viewDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        XsSBL3Actions{
            visible: currentCategory=="Tree"
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: (XsStyleSheet.widgetStdHeight*2 + buttonSpacing*3)
        }

    }

}