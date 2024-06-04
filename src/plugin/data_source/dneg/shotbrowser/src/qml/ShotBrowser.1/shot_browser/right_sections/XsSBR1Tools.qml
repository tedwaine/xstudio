// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0

Item{ id: toolDiv

    property real categoryBtnWidth: btnWidth * 1.3

    clip: true

    ColumnLayout{
        spacing: panelPadding
        width: parent.width
        height: parent.height

        RowLayout{
            spacing: buttonSpacing*2
            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight

            XsSearchButton{ id: filterBtn
                Layout.preferredWidth: isExpanded? btnWidth*5.3: btnWidth
                Layout.maximumWidth: isExpanded? btnWidth*5.3 : btnWidth
                Layout.preferredHeight: parent.height
                isExpanded: false
                hint: "Filter"
                onTextChanged: nameFilter = text

                Connections {
                    target: panel
                    function onNameFilterChanged() {
                        filterBtn.text = nameFilter
                    }
                }
            }
            Item{
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height
            }

            MouseArea {
                Layout.preferredWidth: btnWidth*2.5
                Layout.preferredHeight: parent.height

                hoverEnabled: true

                XsSBCountDisplay{
                    anchors.fill: parent
                    filteredCount: resultsFilteredModel.count
                    totalCount: resultsBaseModel.count == undefined ? "-" : resultsBaseModel.truncated ? resultsBaseModel.count + "+" : resultsBaseModel.count
                }

                XsToolTip {
                    timeout: 0
                    visible: parent.containsMouse
                    text: "Execution time " + resultsBaseModel.executionMilliseconds + " ms."
                }
            }

            XsComboBoxEditable{ id: filterStep
                Layout.minimumWidth: btnWidth*3.5
                Layout.preferredWidth: btnWidth*4
                Layout.preferredHeight: parent.height
                model: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Pipeline Step") : []
                textRole: "nameRole"
                currentIndex: -1
                displayText: currentIndex ==-1 ? "Pipeline Step" : currentText

                onModelChanged: currentIndex = -1

                onCurrentIndexChanged: {
                    if(currentIndex == -1)
                        pipeStep = ""
                }
                onAccepted: {
                    pipeStep = model.get(model.index(currentIndex, 0), "nameRole")
                    toolDiv.forceActiveFocus()
                }

                onActivated: pipeStep = model.get(model.index(currentIndex,0), "nameRole")

                Connections {
                    target: panel
                    function onPipeStepChanged() {
                        filterStep.currentIndex = filterStep.find(pipeStep)
                    }
                }
            }
            XsComboBoxEditable{ id: filterOnDisk
                Layout.minimumWidth: btnWidth*3
                Layout.preferredWidth: btnWidth*3.5
                Layout.preferredHeight: parent.height
                model: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Site") : []
                currentIndex: -1
                textRole: "nameRole"
                displayText: currentIndex==-1? "On Disk" : currentText

                onModelChanged: currentIndex = -1

                onCurrentIndexChanged: {
                    if(currentIndex==-1)
                        onDisk = ""
                }

                onAccepted: {
                    onDisk = model.get(model.index(currentIndex,0), "nameRole")
                    toolDiv.forceActiveFocus()
                }

                onActivated: onDisk = model.get(model.index(currentIndex,0), "nameRole")

                Connections {
                    target: panel
                    function onOnDiskChanged() {
                        filterOnDisk.currentIndex = filterOnDisk.find(onDisk)
                    }
                }

            }
        }
        Rectangle{
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: panelColor
        }

        RowLayout{
            spacing: buttonSpacing
            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight

            XsButtonWithImageAndText{ id: groupBtn
                Layout.fillWidth: true
                Layout.minimumWidth: btnWidth
                Layout.preferredWidth: btnWidth*2.2
                Layout.maximumWidth: btnWidth*2.2
                Layout.preferredHeight: parent.height
                iconSrc: "qrc:///shotbrowser_icons/account_tree.svg"
                iconText: "Group"
                isActive: resultsBaseModel.isGrouped
                visible: resultsBaseModel.canBeGrouped
                onClicked: resultsBaseModel.isGrouped = !resultsBaseModel.isGrouped
            }
            Item{
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height

                XsLabel {
                    text: resultViewTitle
                    // color: XsStyleSheet.hintColor
                    visible: presetsSelectionModel.hasSelection

                    anchors.centerIn: parent
                    width: parent.width - panelPadding*2
                    height: parent.height
                }
            }
            XsSortButton{ id: sortViaNaturalOrderBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                text: "ShotGrid Order"
                isActive: sortByNaturalOrder
                sortIconText: "SG"
                isDescendingOrder: sortByNaturalOrder && !sortInAscending

                onClicked: {
                    if(sortByNaturalOrder && sortInAscending) sortInAscending = false
                    else sortInAscending = true
                    sortByNaturalOrder = true
                }
            }
            XsSortButton{ id: sortViaDateBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                text: "Creation Date"
                isActive: sortByCreationDate
                sortIconSrc: "qrc:///shotbrowser_icons/calendar_month.svg"
                isDescendingOrder: sortByCreationDate && !sortInAscending

                onClicked: {
                    if(sortByCreationDate && sortInAscending) sortInAscending = false
                    else sortInAscending = true
                    sortByCreationDate = true
                }
            }
            XsSortButton{ id: sortViaShotBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                text: "Shot Name"
                isActive: sortByShotName
                sortIconText: "AZ"
                isDescendingOrder: sortByShotName && !sortInAscending

                onClicked: {
                    if(sortByShotName && sortInAscending) sortInAscending = false
                    else sortInAscending = true
                    sortByShotName = true
                }
            }
        }

    }



}