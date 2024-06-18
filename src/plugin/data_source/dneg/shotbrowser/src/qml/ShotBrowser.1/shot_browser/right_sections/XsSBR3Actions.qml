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

    property real itemHeight: XsStyleSheet.widgetStdHeight

    RowLayout {
        anchors.fill: parent
        spacing: buttonSpacing

        XsPrimaryButton{
            Layout.preferredWidth: parent.width / parent.children.length
            Layout.preferredHeight: itemHeight
            text: "Add"
            onClicked: {
                if(resultsBaseModel.groupId != helpers.QVariantFromUuidString("087c4ff5-2da0-4e54-afcf-c7914a247fae"))
                    ShotBrowserHelpers.addToCurrent(resultsSelectionModel.selectedIndexes, false)
                else
                    ShotBrowserHelpers.addSequencesToNewPlaylist(resultsSelectionModel.selectedIndexes)
            }
        }
        XsPrimaryButton{
            Layout.preferredWidth: parent.width / parent.children.length
            Layout.preferredHeight: itemHeight
            text: "Replace"
            onClicked: ShotBrowserHelpers.replaceSelectedResults(resultsSelectionModel.selectedIndexes)
            enabled: resultsBaseModel.groupId != helpers.QVariantFromUuidString("087c4ff5-2da0-4e54-afcf-c7914a247fae")
        }
        XsPrimaryButton{
            Layout.preferredWidth: parent.width / parent.children.length
            Layout.preferredHeight: itemHeight
            text: "Compare"
            enabled: resultsBaseModel.groupId != helpers.QVariantFromUuidString("087c4ff5-2da0-4e54-afcf-c7914a247fae")
            onClicked: ShotBrowserHelpers.compareSelectedResults(resultsSelectionModel.selectedIndexes)
        }
    }
}