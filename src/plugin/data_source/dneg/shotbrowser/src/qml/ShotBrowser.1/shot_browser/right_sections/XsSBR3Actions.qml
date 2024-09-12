// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QuickFuture 1.0
import QuickPromise 1.0
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudio 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

RowLayout {
    spacing: 2
    property int parentWidth: width
    readonly property int cellWidth: parentWidth / children.length

    XsPrimaryButton{
        Layout.fillHeight: true
        Layout.preferredWidth: cellWidth
        text: "Add"
        onClicked: {
            if(resultsBaseModel.groupId != helpers.QVariantFromUuidString("087c4ff5-2da0-4e54-afcf-c7914a247fae"))
                ShotBrowserHelpers.addToCurrent(resultsSelectionModel.selectedIndexes, false)
            else
                ShotBrowserHelpers.addSequencesToNewPlaylist(resultsSelectionModel.selectedIndexes)
        }
    }

    XsPrimaryButton{
        Layout.fillHeight: true
        Layout.preferredWidth: cellWidth
        text: "Replace"
        onClicked: ShotBrowserHelpers.replaceSelectedResults(resultsSelectionModel.selectedIndexes)
        enabled: resultsBaseModel.groupId != helpers.QVariantFromUuidString("087c4ff5-2da0-4e54-afcf-c7914a247fae")
    }

    XsPrimaryButton{
        Layout.fillHeight: true
        Layout.preferredWidth: cellWidth
        text: "Compare"
        enabled: resultsBaseModel.groupId != helpers.QVariantFromUuidString("087c4ff5-2da0-4e54-afcf-c7914a247fae")
        onClicked: ShotBrowserHelpers.compareSelectedResults(resultsSelectionModel.selectedIndexes)
    }
}
