// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.clipboard 1.0


XsPopupMenu {
    id: rightClickMenu
    visible: false

    property var popupSelectionModel
    property var popupDelegateModel

    Clipboard {
       id: clipboard
    }

    XsMenuModelItem {
        text: "Select All"
        menuItemPosition: 1
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
        onActivated:  popupSelectionModel.select(
            helpers.createItemSelectionFromList(ShotBrowserHelpers.getAllIndexes(popupDelegateModel)),
            ItemSelectionModel.Select
        )
    }
    XsMenuModelItem {
        text: "Deselect All"
        menuItemPosition: 2
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
        onActivated: popupSelectionModel.clear()
    }
    XsMenuModelItem {
        text: "Invert Selection"
        menuItemPosition: 3
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
        onActivated: popupSelectionModel.select(
            helpers.createItemSelectionFromList(ShotBrowserHelpers.getAllIndexes(popupDelegateModel)),
            ItemSelectionModel.Toggle
        )
    }
    XsMenuModelItem {
        menuItemType: "divider"
        menuItemPosition: 4
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
    }
    XsMenuModelItem {
        text: "Reveal In ShotGrid"
        menuItemPosition: 5
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.revealInShotgrid(popupSelectionModel.selectedIndexes)
    }
    XsMenuModelItem {
        text: "Reveal In Ivy"
        menuItemPosition: 6
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.revealInIvy(popupSelectionModel.selectedIndexes)
    }

    XsMenuModelItem {
        text: "Copy JSON"
        menuItemPosition: 6.5
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
        onActivated: clipboard.text = JSON.stringify(ShotBrowserHelpers.getJSON(popupSelectionModel.selectedIndexes))
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuItemPosition: 7
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
    }
    // XsMenuModelItem {
    //     text: "Transfer Selected"
    //     menuItemPosition: 10
    //     menuPath: ""
    //     menuModelName: rightClickMenu.menu_model_name
    //     onActivated: {}
    // }


    XsMenuModelItem {
        text: "To Chennai"
        menuItemPosition: 1
        menuPath: "Transfer Selected"
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.transfer("chn", popupSelectionModel.selectedIndexes)
    }
    XsMenuModelItem {
        text: "To London"
        menuItemPosition: 2
        menuPath: "Transfer Selected"
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.transfer("lon", popupSelectionModel.selectedIndexes)
    }
    XsMenuModelItem {
        text: "To Montreal"
        menuItemPosition: 2
        menuPath: "Transfer Selected"
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.transfer("mtl", popupSelectionModel.selectedIndexes)
    }
    XsMenuModelItem {
        text: "To Mumbai"
        menuItemPosition: 2
        menuPath: "Transfer Selected"
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.transfer("mum", popupSelectionModel.selectedIndexes)
    }
    XsMenuModelItem {
        text: "To Sydney"
        menuItemPosition: 2
        menuPath: "Transfer Selected"
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.transfer("syd", popupSelectionModel.selectedIndexes)
    }
    XsMenuModelItem {
        text: "To Vancouver"
        menuItemPosition: 2
        menuPath: "Transfer Selected"
        menuModelName: rightClickMenu.menu_model_name
        onActivated: ShotBrowserHelpers.transfer("van", popupSelectionModel.selectedIndexes)
    }
    XsMenuModelItem {
        menuItemType: "divider"
        menuItemPosition: 3
        menuPath: ""
        menuModelName: rightClickMenu.menu_model_name
    }
    XsMenuModelItem {
        text: "Open Transfer Tool"
        menuItemPosition: 4
        menuPath: "Transfer Selected"
        menuModelName: rightClickMenu.menu_model_name
        onActivated: helpers.startDetachedProcess("dnenv-do", [helpers.getEnv("SHOW"), helpers.getEnv("SHOT"), "--", "maketransfer"])
    }
}
