// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0

Item{
    clip: true

    property var presetsFilterModel: {
        if(!ShotBrowserEngine.ready)
            return null
        else {
            if(currentCategory == "Tree") return treeModel
            else if(currentCategory == "Recent") return recentModel
            else return menuModel
        }
    }

    XsSplitView { id: viewDiv

        anchors.fill: parent
        spacing: currentCategory == "Tree"? panelPadding : 0
        thumbWidth: currentCategory == "Tree"? panelPadding/2 : 0

        XsSBL2V1Tree{ id: treeView
            SplitView.preferredWidth: prefs.leftPanelWidth
            SplitView.fillHeight: true

            visible: currentCategory == "Tree"
            onWidthChanged: {
                if(SplitView.view.resizing && currentCategory == "Tree")
                    prefs.leftPanelWidth = width
            }
        }

        XsSBL2V2Presets{ id: presetsView
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            visible: currentCategory != "Tree" || sequenceTreeShowPresets
        }
    }
}