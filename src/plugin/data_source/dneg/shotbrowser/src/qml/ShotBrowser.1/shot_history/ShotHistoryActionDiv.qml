// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0
import QuickFuture 1.0
import QuickPromise 1.0
import xstudio.qml.helpers 1.0

Item{id: buttonsDiv

    property int actionButtonCount: 4
    property real actionButtonSpacing: 1
    property real actionButtonWidth: ( row.width - actionButtonSpacing*(actionButtonCount-1) ) / actionButtonCount
    property real actionButtonHeight: XsStyleSheet.widgetStdHeight

    // enabled: isPanelEnabled

    RowLayout{ id: row
        x: panelPadding
        width: parent.width - (panelPadding*2)
        height: actionButtonHeight
        anchors.centerIn: parent
        spacing: actionButtonSpacing

        XsPrimaryButton{
            text: "Add"
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: parent.height
            onClicked: ShotBrowserHelpers.addToCurrentPlaylist(resultsSelectionModel.selectedIndexes)
        }
        XsPrimaryButton{
            text: "Replace"
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: parent.height
            onClicked: ShotBrowserHelpers.replaceSelectedResults(resultsSelectionModel.selectedIndexes)
        }
        XsPrimaryButton{
            text: "Compare"
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: parent.height
            onClicked: ShotBrowserHelpers.compareSelectedResults(resultsSelectionModel.selectedIndexes)
        }
        XsPrimaryButton{
            text: "Add To Sequence"
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: parent.height
        }

    }

}