// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0

Item{
    clip: true

    property var presetsFilterModel: ShotBrowserEngine.presetModels.recent

    property real treeTabWidth: visibleWidth
    property real menusRecentTabWidth: visibleWidth

    property real leftSecWidth: parent.parent.width
    onLeftSecWidthChanged: {
        if(currentCategory == "Tree") {
            treeTabWidth = leftSecWidth
        }
        else if(currentCategory == "Recent" || currentCategory == "Menus") {
            menusRecentTabWidth = leftSecWidth
        }
    }

    property string selectedCategory: currentCategory
    onSelectedCategoryChanged: {
        if(currentCategory == "Tree") {
            parent.parent.SplitView.preferredWidth = treeTabWidth
        }
        else if(currentCategory == "Recent" || currentCategory == "Menus") {
            parent.parent.SplitView.preferredWidth = menusRecentTabWidth
        }
    }


    XsSplitView { id: viewDiv

        anchors.fill: parent
        spacing: currentCategory == "Tree"? panelPadding : 0
        thumbWidth: currentCategory == "Tree"? panelPadding/2 : 0

        XsSBL2V1Tree{ id: treeView
            SplitView.preferredWidth: treeView.actualWidth
            SplitView.fillHeight: true

            visible: currentCategory == "Tree"
            property real actualWidth: parent.width/2
            onWidthChanged: if(currentCategory == "Tree") treeView.actualWidth = width
        }

        XsSBL2V2Presets{ id: presetsView
            SplitView.fillWidth: true
            SplitView.fillHeight: true

            property var selectedCategory: currentCategory
            onSelectedCategoryChanged: {
                if(currentCategory == "Tree") presetsFilterModel = ShotBrowserEngine.presetModels.tree
                else if(currentCategory == "Recent") presetsFilterModel = ShotBrowserEngine.presetModels.recent
                else presetsFilterModel = ShotBrowserEngine.presetModels.menus
            }
        }

    }

}