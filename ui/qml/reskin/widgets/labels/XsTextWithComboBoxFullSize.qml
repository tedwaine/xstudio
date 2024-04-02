// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0


Item{ id: widget

    property alias text: labelDiv.text
    // property alias value: valueDiv.model[currentIndex]
    property alias valueDiv: valueDiv

    property alias model: valueDiv.model
    property alias currentIndex: valueDiv.currentIndex

    ColumnLayout{
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 5
    
        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height/3

            XsText{ id: labelDiv
                anchors.fill: parent
                horizontalAlignment: Text.AlignLeft
                text: ""
            }
        }
        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height/1.5
            
            RowLayout{
                anchors.fill: parent
                spacing: 1

                Item{
                    Layout.preferredWidth: 20
                    Layout.fillHeight: true
                }
                XsComboBox{ id: valueDiv
                    Layout.fillWidth: true
                    Layout.fillHeight: true
    
                    onCurrentIndexChanged: {
                        widget.currentIndexChanged()
                    }
                }
            }
        }
    }

}