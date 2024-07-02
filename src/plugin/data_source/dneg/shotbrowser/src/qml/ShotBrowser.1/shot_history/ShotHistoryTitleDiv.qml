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
        id: filterModel
        showHidden: false
        onlyShowFavourite: true
        sourceModel: ShotBrowserEngine.presetsModel
    }

    DelegateModel {
        id: groupModel
        property var srcModel: filterModel
        onSrcModelChanged: model = srcModel

        delegate: Item {
            width: ListView.view.width / ListView.view.count
            height: titleButtonHeight

            XsPrimaryButton{
                width: parent.width  - titleButtonSpacing
                height: titleButtonHeight
                text: nameRole
                isActive: activeScopeIndex == groupModel.srcModel.mapToSource(groupModel.modelIndex(index))

                onClicked: {
                    activateScope(groupModel.srcModel.mapToSource(groupModel.modelIndex(index)))
                }
            }
        }
    }

    function populateModels() {
        groupModel.rootIndex = helpers.makePersistent(filterModel.mapFromSource(ShotBrowserEngine.presetsModel.searchRecursive(
            "c5ce1db6-dac0-4481-a42b-202e637ac819", "idRole", ShotBrowserEngine.presetsModel.index(-1, -1), 0, 1
        )))
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

    XsPrimaryButton{ id: updateScopeBtn
        x: panelPadding
        width: 40
        height: parent.height - (panelPadding*2)
        anchors.verticalCenter: parent.verticalCenter
        imgSrc: isPanelEnabled && !isPaused ? "qrc:///shotbrowser_icons/lock_open.svg" : "qrc:///shotbrowser_icons/lock.svg"
        // text: isPanelEnabled? "ON" : "OFF"
        isActive: !isPanelEnabled || isPaused
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
                enabled: isPanelEnabled && !isPaused
            }
        }
        RowLayout{
            width: parent.width
            height: titleButtonHeight
            spacing: 0

            XsSearchButton{ id: filterBtn
                Layout.fillWidth: true
                Layout.fillHeight: true
                isExpanded: true
                hint: "Filter"
                buttonWidth: scopeTxt.width
                enabled: isPanelEnabled && !isPaused

                onTextChanged: nameFilter = text

                Connections {
                    target: panel
                    function onNameFilterChanged() {
                        filterBtn.text = nameFilter
                    }
                }
            }

            XsComboBoxEditable{ id: filterSentTo
                Layout.fillHeight: true
                Layout.minimumWidth: titleButtonHeight * 3
                Layout.preferredWidth: titleButtonHeight * 3

                enabled: isPanelEnabled && !isPaused

                model: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Sent To") : []
                currentIndex: -1
                textRole: "nameRole"
                displayText: currentIndex==-1? "Sent To" : currentText

                onModelChanged: currentIndex = -1

                onCurrentIndexChanged: {
                    if(currentIndex==-1)
                        sentTo = ""
                }

                onAccepted: {
                    sentTo = model.get(model.index(currentIndex,0), "nameRole")
                    toolDiv.forceActiveFocus()
                }

                onActivated: sentTo = model.get(model.index(currentIndex,0), "nameRole")

                Connections {
                    target: panel
                    function onSentToChanged() {
                        filterSentTo.currentIndex = filterSentTo.find(sentTo)
                    }
                }
            }
        }
    }
}