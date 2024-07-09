// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
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
    thumbWidth: currentCategory == "Tree"? panelPadding/2 : 0

    XsSBL2V1Tree{ id: treeView
        SplitView.preferredWidth: prefs.leftPanelWidth-8
        SplitView.fillHeight: true

        visible: currentCategory == "Tree"
        onWidthChanged: {
            if(SplitView.view.resizing && currentCategory == "Tree")
                prefs.leftPanelWidth = width+8
        }
    }

    XsSBL2V2Presets{ id: presetsView
        SplitView.fillWidth: true
        SplitView.fillHeight: true
        visible: currentCategory != "Tree" || sequenceTreeShowPresets
    }
}
