// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import xStudio 1.0
import Grading 2.0

Item{ id: presetView

    property real flickageMminimumWidth: 160 + (206*3) + 2


    XsGradientRectangle{
        anchors.fill: parent
    }

    Flickable{
        id: flickable
        anchors.fill: parent
        contentWidth: layout.width
        contentHeight: layout.height

        ScrollBar.horizontal: XsScrollBar {
            visible: flickable.width < flickable.contentWidth
        }
        ScrollBar.vertical: XsScrollBar {
            visible: flickable.height < flickable.contentHeight
        }



        RowLayout {
            id: layout
            width: presetView.width
            height: presetView.height
            spacing: 0

            GTSliderItem {
                Layout.fillWidth: true
                Layout.minimumWidth: 165
                Layout.fillHeight: true
            }

            Repeater{
                model: attrs.grading_wheels_model

                GTWheelItem {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 138
                    Layout.preferredWidth: 188
                    Layout.fillHeight: true
                }
            }
        }
    }

}