// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0

Item{ id: contentDiv

    Rectangle{
        x: panelPadding
        width: panel.width-(x*2)
        height: parent.height
        color: panelColor

        XsLabel {
            text: "Select the 'Scope' and the 'Note Type' to view the Note History."
            color: XsStyleSheet.hintColor
            visible: !activeScopeIndex.valid || !activeTypeIndex.valid

            anchors.centerIn: parent
            width: parent.width - panelPadding*2
            height: parent.height - panelPadding*2

            font.pixelSize: XsStyleSheet.fontSize*1.2
            font.weight: Font.Medium
        }

        XsLabel {
            text: !queryRunning ? "No Results Found" : ""
            // text: isPaused ? "Updates Paused" : !queryRunning ? "No Results Found" : ""
            color: XsStyleSheet.hintColor
            visible: dataModel && activeScopeIndex.valid && activeTypeIndex.valid && !dataModel.count //#TODO

            anchors.centerIn: parent
            width: parent.width - panelPadding*2
            height: parent.height - panelPadding*2

            font.pixelSize: XsStyleSheet.fontSize*1.2
            font.weight: Font.Medium
        }

        DelegateModel {
            id: chooserModel
            model: dataModel
            delegate: NotesHistoryListDelegate{

                width: ListView.view.listItemWidth
                height: ListView.view.listItemHeight
                listSpacing: ListView.view.listItemSpacing
                delegateModel: chooserModel
                popupMenu: resultPopup
            }
        }

        XsListView{ id: list
            width: parent.width - panelPadding*2
            height: parent.height - panelPadding*2
            anchors.centerIn: parent

            property real listItemSpacing: panelPadding
            property real listItemWidth: width
            property real listItemHeight: (XsStyleSheet.widgetStdHeight*8) + (1*7)

            model: chooserModel
        }
    }
}