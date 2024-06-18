// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

Item{id: titleDiv

    property int titleButtonCount: 4
    property real titleButtonSpacing: 1
    property real titleButtonHeight: XsStyleSheet.widgetStdHeight+4

    ShotBrowserPresetFilterModel {
        id: scopeFilterModel
        showHidden: false
        onlyShowFavourite: true
        filterUserData: "scope"

        sourceModel: ShotBrowserEngine.presetsModel
    }

    ShotBrowserPresetFilterModel {
        id: typeFilterModel
        showHidden: false
        onlyShowFavourite: true
        filterUserData: "type"

        sourceModel: ShotBrowserEngine.presetsModel
    }


    DelegateModel {
        id: scopeGroupModel

        property var srcModel: scopeFilterModel
        onSrcModelChanged: model = srcModel

        delegate: Item {
            width: ListView.view.width / ListView.view.count
            height: titleButtonHeight

            XsPrimaryButton{
                width: parent.width  - titleButtonSpacing
                height: titleButtonHeight
                text: nameRole
                isActive: activeScopeIndex == scopeGroupModel.srcModel.mapToSource(scopeGroupModel.modelIndex(index))

                onClicked: {
                    activateScope(scopeGroupModel.srcModel.mapToSource(scopeGroupModel.modelIndex(index)))
                }
            }
        }
    }

    DelegateModel {
        id: typeGroupModel

        property var srcModel: typeFilterModel
        onSrcModelChanged: model = srcModel

        delegate: Item {
            width: ListView.view.width / ListView.view.count
            height: titleButtonHeight

            XsPrimaryButton{
                width: parent.width  - titleButtonSpacing
                height: titleButtonHeight
                text: nameRole
                isActive: activeTypeIndex == typeGroupModel.srcModel.mapToSource(typeGroupModel.modelIndex(index))

                onClicked: {
                    activateType(typeGroupModel.srcModel.mapToSource(typeGroupModel.modelIndex(index)))
                }
            }
        }
    }

    function populateModels() {
        typeGroupModel.rootIndex = helpers.makePersistent(typeFilterModel.mapFromSource(ShotBrowserEngine.presetsModel.searchRecursive(
            "aac8207e-129d-4988-9e05-b59f75ae2f75", "idRole", ShotBrowserEngine.presetsModel.index(-1, -1), 0, 1
        )))
        typeList.model = typeGroupModel

        scopeGroupModel.rootIndex = helpers.makePersistent(scopeFilterModel.mapFromSource(ShotBrowserEngine.presetsModel.searchRecursive(
            "aac8207e-129d-4988-9e05-b59f75ae2f75", "idRole", ShotBrowserEngine.presetsModel.index(-1, -1), 0, 1
        )))
        scopeList.model = scopeGroupModel
    }

    Component.onCompleted: {
        if(ShotBrowserEngine.ready)
            populateModels()
    }

    Connections {
        target: ShotBrowserEngine
        function onReadyChanged() {
            if(ShotBrowserEngine.ready)
                populateModels()
        }
    }

    XsPrimaryButton{ id: updateScopeBtn
        x: panelPadding
        width: 40
        height: parent.height - (panelPadding*2)
        anchors.verticalCenter: parent.verticalCenter
        imgSrc: isPanelEnabled? "qrc:///shotbrowser_icons/lock_open.svg" : "qrc:///shotbrowser_icons/lock.svg"
        // text: isPanelEnabled? "ON" : "OFF"
        isActive: !isPanelEnabled
        onClicked: {
            isPanelEnabled = !isPanelEnabled
        }
    }

    ColumnLayout{ id: col
        width: (parent.width - updateScopeBtn.width) - (panelPadding*2)
        height: parent.height - (panelPadding*2)
        spacing: titleButtonSpacing
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: updateScopeBtn.right
        anchors.leftMargin: titleButtonSpacing

        RowLayout{
            width: parent.width
            height: titleButtonHeight
            spacing: 0

            XsText{ id: scopeTxt
                Layout.preferredWidth: (textWidth + panelPadding*3)
                Layout.preferredHeight: titleButtonHeight
                text: "Scope: "
            }

            XsListView{ id: scopeList
                Layout.fillWidth: true
                Layout.preferredHeight: titleButtonHeight

                orientation: ListView.Horizontal
                enabled: isPanelEnabled
                model: []
            }
        }

        RowLayout{
            width: parent.width
            height: titleButtonHeight
            spacing: 0

            XsText{ id: typeTxt
                Layout.preferredWidth: scopeTxt.width
                Layout.preferredHeight: titleButtonHeight
                text: "Type: "
            }

            XsListView{ id: typeList
                Layout.fillWidth: true
                Layout.preferredHeight: titleButtonHeight

                orientation: ListView.Horizontal
                enabled: isPanelEnabled
                model: []
            }
        }
    }
}