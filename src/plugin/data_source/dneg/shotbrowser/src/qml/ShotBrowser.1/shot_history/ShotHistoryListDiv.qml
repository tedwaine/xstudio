// SPDX-License-Identifier: Apache-2.0

import QtQuick

import QtQuick.Layouts


import xStudio 1.0
import ShotBrowser 1.0

XsListView{ id: list
    spacing: panelPadding
    property int rightSpacing: list.height < list.contentHeight ? 16 : 0
    Behavior on rightSpacing {NumberAnimation {duration: 150}}

    XsLabel {
        text: "Select the 'Scope' to view the Shot History."
        color: XsStyleSheet.hintColor
        visible: !activeScopeIndex.valid //#TODO

        anchors.fill: parent

        font.pixelSize: XsStyleSheet.fontSize*1.2
        font.weight: Font.Medium
    }

    XsLabel {
        text: !queryRunning ? "No Results Found" : ""
        // text: isPaused ? "Updates Paused" : !queryRunning ? "No Results Found" : ""
        color: XsStyleSheet.hintColor
        visible: dataModel &&  activeScopeIndex.valid && !dataModel.count //#TODO

        anchors.fill: parent

        font.pixelSize: XsStyleSheet.fontSize*1.2
        font.weight: Font.Medium
    }

    model: DelegateModel {
        id: chooserModel
        model: dataModel
        delegate: ShotHistoryListDelegate{
            width: list.width - rightSpacing
            height: XsStyleSheet.widgetStdHeight * 4
            delegateModel: chooserModel
            popupMenu: resultPopup
        }
    }
}

