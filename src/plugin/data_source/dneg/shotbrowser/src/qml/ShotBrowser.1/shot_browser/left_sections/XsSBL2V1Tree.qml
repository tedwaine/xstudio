// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudio 1.0
import ShotBrowser 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0


Item{

    XsGradientRectangle{
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.rightMargin: panelPadding
        spacing: panelPadding

        Rectangle{
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: panelColor
        }

        RowLayout { id: headerDiv
            Layout.fillWidth: true;
            Layout.preferredHeight: btnHeight
            spacing: buttonSpacing
            z: 1

            XsSBTreeSearchButton{ id: searchBtn
                Layout.fillWidth: isExpanded
                Layout.minimumWidth: btnWidth
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                isExpanded: false
                hint: "Search..."
                model: ShotBrowserFilterModel {
                    sourceModel: ShotBrowserEngine.presetsModel.termModel("ShotSequenceList", "", projectId)
                }
                onIndexSelected: {
                    // possibility of id collisions ?
                    let mid = index.model.get(index, "idRole")
                    let ti = sequenceSelectionModel.model.searchRecursive(mid, "idRole")
                    sequenceSelectionModel.select(ti, ItemSelectionModel.ClearAndSelect)
                }
            }
            Item{
                Layout.fillWidth: !searchBtn.isExpanded
                Layout.preferredWidth: searchBtn.isExpanded? buttonSpacing : buttonSpacing*8
                Layout.preferredHeight: parent.height
            }
            XsButtonWithImageAndText{ id: liveLinkBtn
                Layout.fillWidth: !searchBtn.isExpanded
                Layout.minimumWidth: btnWidth
                Layout.preferredWidth: searchBtn.isExpanded? btnWidth : btnWidth*2
                Layout.maximumWidth: btnWidth*2
                Layout.preferredHeight: parent.height
                iconSrc: "qrc:/icons/link.svg"
                iconText: "Link"
                isActive: sequenceTreeLiveLink && !isPaused
                onClicked: {
                    // searchBtn.isExpanded = false
                    sequenceTreeLiveLink  = !sequenceTreeLiveLink
                }
            }
            Item{
                Layout.fillWidth: !searchBtn.isExpanded
                Layout.preferredWidth: searchBtn.isExpanded? buttonSpacing : buttonSpacing*8
                Layout.preferredHeight: parent.height
            }
            XsPrimaryButton{ id: filterBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/filter.svg"
                isActive: sequenceModel && sequenceModel.hideStatus.length
                onClicked: {
                    // searchBtn.isExpanded = false
                    if(shotFilterPopup.visible) {
                        shotFilterPopup.visible = false
                    } else {
                        shotFilterPopup.showMenu(
                            filterBtn,
                            width/2,
                            height/2);
                    }
                }
            }
        }

        XsPopupMenu {
            id: shotFilterPopup
            menu_model_name: "shot_filter_popup"
            visible: false

            closePolicy: filterBtn.hovered ? Popup.CloseOnEscape :  Popup.CloseOnEscape | Popup.CloseOnPressOutside

            Repeater {
                model:  DelegateModel {
                    property var notifyModel: ShotBrowserEngine.presetsModel.termModel("Shot Status")
                    model: notifyModel
                    delegate :
                        Item {
                            XsMenuModelItem {
                                text: nameRole
                                menuItemType: "toggle"
                                menuPath: ""
                                menuItemPosition: index
                                menuModelName: shotFilterPopup.menu_model_name
                                isChecked: sequenceModel && (sequenceModel.hideStatus.includes(nameRole) || sequenceModel.hideStatus.includes(idRole))
                                onActivated: {
                                    if(isChecked) {
                                        sequenceModel.hideStatus = Array.from(sequenceModel.hideStatus).filter(r => r !== idRole && r !== nameRole)
                                    } else {
                                        let tmp = sequenceModel.hideStatus
                                        tmp.push(idRole)
                                        tmp.push(nameRole)
                                        sequenceModel.hideStatus = tmp
                                    }
                                }
                            }
                        }
                }
            }
        }

        Rectangle{
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            color: panelColor

            Flickable {
                id: sequenceTreeView
                anchors.fill: parent
                clip: true

                contentWidth: width
                contentHeight: tree.height

                ScrollBar.vertical: XsScrollBar{visible: sequenceTreeView.height < sequenceTreeView.contentHeight}

                XsTimer {
                    id: delayTimer
                }

                function jumpTo(item) {
                    delayTimer.setTimeout(function() {
                        jumpToReal(item)
                    }, 200)
                }

                function jumpToReal(item) {
                    // need to wait for expansion of parents.

                    let r = mapFromItem(item, 0,0, item.width, item.height)
                    let my = contentY+r.y

                    // jump up
                    if(my < contentY) {
                        contentY = my - height + r.height
                    } else if(my + btnHeight - 4 > contentY + sequenceTreeView.parent.height) {
                        contentY = my
                    }
                }

                XsSBTreeView{
                    id: tree
                    width: sequenceTreeView.width
                    treeSequenceModel: sequenceModel
                    treeSequenceSelectionModel: sequenceSelectionModel
                    treeSequenceExpandedModel: sequenceExpandedModel
                    onTreeSequenceModelChanged: {
                        if(treeSequenceModel != null)
                            treeRootIndex = treeSequenceModel.index(-1,-1)
                    }
                }
            }
        }
    }
}