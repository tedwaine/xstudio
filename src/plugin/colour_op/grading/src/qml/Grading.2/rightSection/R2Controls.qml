// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0 //for RadialGradient

import xStudioReskin 1.0
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
        clip: true

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
                Layout.minimumWidth: 160
                Layout.fillHeight: true
            }

            Repeater{
                model: attrs.grading_wheels_model

                GTWheelItem {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 128
                    Layout.preferredWidth: 188 
                    Layout.fillHeight: true
                }
            }
        }
    }

}