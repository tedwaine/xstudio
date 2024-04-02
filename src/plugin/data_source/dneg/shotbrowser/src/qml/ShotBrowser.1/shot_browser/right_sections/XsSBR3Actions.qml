// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QuickFuture 1.0
import QuickPromise 1.0
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

Rectangle{
    color: "transparent" //panelColor

    property int actionButtonCount: 4
    property real actionButtonWidth: ( width - buttonSpacing*(actionButtonCount-1) ) / actionButtonCount

    property real itemHeight: XsStyleSheet.widgetStdHeight

    RowLayout {
        anchors.fill: parent
        spacing: buttonSpacing

        XsPrimaryButton{
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: itemHeight
            imgSrc: ""
            onClicked: ShotBrowserHelpers.addToCurrentPlaylist(resultsSelectionModel.selectedIndexes)
            text: "Add"
        }
        XsPrimaryButton{
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: itemHeight
            imgSrc: ""
            text: "Replace"
            onClicked: ShotBrowserHelpers.replaceSelectedResults(resultsSelectionModel.selectedIndexes)
        }
        XsPrimaryButton{
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: itemHeight
            imgSrc: ""
            text: "Compare"
            onClicked: ShotBrowserHelpers.compareSelectedResults(resultsSelectionModel.selectedIndexes)
        }
        XsPrimaryButton{
            Layout.preferredWidth: actionButtonWidth
            Layout.preferredHeight: itemHeight
            imgSrc: ""
            text: "Add to Sequence"
        }

    }

}