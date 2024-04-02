// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0


Item{ id: widget

    property alias text: labelDiv.text
    property alias checked: checkBoxDiv.checked

    RowLayout{ id: row
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 10
        
        XsCheckBox { id: checkBoxDiv
            Layout.preferredWidth: height - parent.spacing
            Layout.fillHeight: true
            forcedHover: labelMArea.containsMouse
            padding: 0
            topInset: 0
            bottomInset: 0
            checked: false
            onClicked: {

            }
        }
        XsText{ id: labelDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            text: ""
            horizontalAlignment: Text.AlignLeft
            opacity: checked? 1 : 0.7

            MouseArea{ id: labelMArea
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    checked = !checked
                }
            }
        }

    }

}