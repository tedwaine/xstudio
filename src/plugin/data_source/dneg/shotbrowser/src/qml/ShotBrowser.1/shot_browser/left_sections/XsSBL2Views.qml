// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import xStudio 1.0
import ShotBrowser 1.0


XsSplitView { id: viewDiv
    property var presetsFilterModel: {
        if(!ShotBrowserEngine.ready)
            return null
        else {
            if(currentCategory == "Tree") return treeModel
            else if(currentCategory == "Recent") return recentModel
            else return menuModel
        }
    }

    spacing: currentCategory == "Tree"? panelPadding : 0
    thumbWidth: currentCategory == "Tree"? panelPadding / 2 : 0

    XsSBL2V1Tree{ id: treeView
        SplitView.minimumWidth: main_split.minimumTreeWidth
        SplitView.preferredWidth: prefs.treePanelWidth
        SplitView.fillHeight: true

        visible: currentCategory == "Tree"

        onWidthChanged: {
            if(SplitView.view.resizing) {
                prefs.treePanelWidth = width
            }
        }
    }

    XsSBL2V2Presets{ id: presetsView
        SplitView.fillWidth: true
        SplitView.fillHeight: true
        SplitView.minimumWidth: 140

        visible: currentCategory != "Tree" || sequenceTreeShowPresets
    }
}
