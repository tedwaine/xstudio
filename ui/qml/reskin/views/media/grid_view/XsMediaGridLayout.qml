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

GridView {

    Layout.fillWidth: true
    Layout.fillHeight: true

    cacheBuffer: 80
    boundsBehavior: Flickable.StopAtBounds

    cellWidth: 200
    cellHeight: cellWidth*9/16

    XsMediaListModelData {
        id: mediaListModelData
        delegate: chooser
    }

    property alias mediaListModelData: mediaListModelData

    model: mediaListModelData

    DelegateChooser {

        id: chooser
        role: "typeRole"

        DelegateChoice {

            roleValue: "Media";

            XsMediaGridItemDelegate {
                Layout.preferredWidth: 40
                property var media_item_model_index: helpers.makePersistent(theSessionData.index(index, 0, mediaListModelData.rootIndex))
            }

        }
    }

}
