// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0

Item{id: titleDiv

    property int titleButtonCount: 4
    property real titleButtonSpacing: 1
    property real titleButtonHeight: XsStyleSheet.widgetStdHeight+4


    DelegateModel {
        id: groupModel

        // this is required as "model" doesn't issue notifications on change
        property var notifyModel: ShotBrowserEngine.presetsModel

        // our model is the main sessionData instance
        model: notifyModel

        delegate: Item {
            width: ListView.view.width / ListView.view.count
            height: titleButtonHeight

            XsPrimaryButton{
                width: parent.width  - titleButtonSpacing
                height: titleButtonHeight
                text: nameRole
                isActive: activeScopeIndex == groupModel.modelIndex(index)

                onClicked: {
                    activateScope(groupModel.modelIndex(index))
                }
            }
        }
    }

    function populateModels() {
        groupModel.rootIndex = ShotBrowserEngine.presetsModel.searchRecursive(
            "c5ce1db6-dac0-4481-a42b-202e637ac819", "idRole", ShotBrowserEngine.presetsModel.index(-1, -1), 0, 1
        )
        scopeList.model = groupModel
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



    ColumnLayout{ id: col
        x: panelPadding
        width: parent.width - (panelPadding*2)
        height: parent.height - (panelPadding*2)
        spacing: titleButtonSpacing
        anchors.verticalCenter: parent.verticalCenter

        RowLayout{
            width: parent.width
            height: titleButtonHeight
            spacing: 0

            XsPrimaryButton{ id: updateScopeBtn
                Layout.preferredWidth: 40
                Layout.preferredHeight: titleButtonHeight
                imgSrc: isPanelEnabled? "qrc:///shotbrowser_icons/lock_open.svg" : "qrc:///shotbrowser_icons/lock.svg"
                // text: isPanelEnabled? "ON" : "OFF"
                isActive: !isPanelEnabled
                onClicked: {
                    isPanelEnabled = !isPanelEnabled
                }
            }

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
            }
        }
    }
}