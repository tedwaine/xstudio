// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0

Item{

    property bool isActive: false

    property real listItemSpacing: 1
    property real borderWidth: 1

    property real itemSpacing: listItemSpacing
    property real itemHeight: 20 

    property color highlightColor: palette.highlight
    property color bgColorNormal: XsStyleSheet.widgetBgNormalColor

    property bool isHovered: mArea.containsMouse || 
        sec1.isHovered ||
        sec2.isHovered ||
        sec3.isHovered
    
    Rectangle{ 
        
        id: frame
        width: parent.width
        height: parent.height - listItemSpacing
        color: "transparent"
        border.color: isHovered? highlightColor : bgColorNormal
        border.width: borderWidth

        MouseArea{ id: mArea
            anchors.fill: parent
            hoverEnabled: true
        }

        RowLayout{
            anchors.fill: parent
            anchors.margins: borderWidth
            spacing: 1

            XsNoteSection1{ id: sec1
                Layout.fillHeight: true
                Layout.preferredWidth: 150
            }
            XsNoteSection2{ id: sec2
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            XsNoteSection3{ id: sec3
                Layout.fillHeight: true
                Layout.preferredWidth: 200
            }

        }


    }

}