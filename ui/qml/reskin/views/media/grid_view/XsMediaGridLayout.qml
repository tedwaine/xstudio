// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

import xstudio.qml.session 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0

import "./delegates"

Item {

    Layout.fillWidth: true
    Layout.fillHeight: true

    property real cellSize: 200

    Rectangle{ id: resultsBg
        anchors.fill: parent
        color: XsStyleSheet.panelBgColor
        z: -1
    }

    XsMediaListModelData {
        id: mediaListModelData
    }
    
    XsGridView {

        width: parent.width - x*2
        height: parent.height - y*2
        x: 10
        y: 10

        cellWidth: cellSize
        cellHeight: cellWidth*9/16
        clip: true
        cacheBuffer: 80
        boundsBehavior: Flickable.StopAtBounds

        // displaced: Transition{
        //     NumberAnimation{ properties: "x,y"; duration: 500 }
        // }
        

        model: mediaListModelData.model

        delegate: XsMediaGridItemDelegate {
            Layout.preferredWidth: 40
            property var media_item_model_index: helpers.makePersistent(theSessionData.index(index, 0, mediaListModelData.rootIndex))
        }
    }

}
