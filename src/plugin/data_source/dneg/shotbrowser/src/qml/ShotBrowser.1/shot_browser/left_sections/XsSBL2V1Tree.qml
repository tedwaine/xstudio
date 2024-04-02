// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0

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
                Layout.preferredWidth: isExpanded? expandedWidth : btnWidth
                Layout.preferredHeight: parent.height
                expandedWidth: btnWidth*3
                isExpanded: false
                hint: "Search..."
                model: ShotBrowserFilterModel {
                    sourceModel: ShotBrowserEngine.presetsModel.termModel("ShotSequenceList", "", projectPref.value)
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
                isActive: sequenceTreeLiveLink
                onClicked: {
                    searchBtn.isExpanded = false
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
                isActive: sequenceModel && !sequenceModel.showOmit
                onClicked: {
                    searchBtn.isExpanded = false
                    sequenceModel.showOmit = !sequenceModel.showOmit
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
                    } else if(my + r.height > contentY + height) {
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