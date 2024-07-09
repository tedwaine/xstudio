// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0

Rectangle{
    color: "transparent"

    property bool isHovered: false

    ColumnLayout {
        anchors.fill: parent
        spacing: itemSpacing

        ShotHistoryTextRow{ id: authorDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: authorRole
            textColor: palette.text
        }
        ShotHistoryTextRow{ id: dateDiv
            Layout.fillWidth: true
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight
            // text: createdDateRole

            property var dateFormatted: createdDateRole.toLocaleString().split(" ")
            // property var timeFormatted: dateFormatted[4].split(":")

            text: typeof dateFormatted !== 'undefined'? dateFormatted[1].substr(0,3)+" "+dateFormatted[2]+" "+dateFormatted[3] : ""
        }
        ShotHistoryTextRow{ id: frameRangeDiv
            Layout.fillWidth: true
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight
            text: frameRangeRole
        }

    }

}