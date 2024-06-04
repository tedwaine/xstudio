// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QuickFuture 1.0

import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0

Item{ id: thisItem

    property bool isActive: false
    property bool isSelected: resultsSelectionModel.isSelected(delegateModel.modelIndex(index))

    property int modelDepth: 0

    property real borderWidth: 1

    property real listSpacing: panelPadding
    property real itemSpacing: 1
    property real itemHeight: XsStyleSheet.widgetStdHeight - 2

    property real textSize: XsStyleSheet.fontSize

    property color hintColor: XsStyleSheet.hintColor
    property color highlightColor: palette.highlight
    property color bgColorNormal: XsStyleSheet.widgetBgNormalColor

    property var delegateModel: null
    property var popupMenu: null
    property bool groupingEnabled: false

    required property string nameRole
    required property string frameRangeRole
    required property string pipelineStepRole
    required property string pipelineStatusFullRole
    required property string authorRole
    required property string thumbRole
    required property string clientFilenameRole

    required property int onSiteChn
    required property int onSiteLon
    required property int onSiteMtl
    required property int onSiteMum
    required property int onSiteSyd
    required property int onSiteVan
    required property int index

    required property int noteCountRole

    required property var submittedToDailiesRole
    required property var dateSubmittedToClientRole


    required property var createdDateRole

    property bool isHovered: mArea.containsMouse ||
        sec1.isHovered ||
        sec2.isHovered ||
        sec3.isHovered ||
        versionArrowBtn.hovered

    Connections {
        target: resultsSelectionModel
        function onSelectionChanged(selected, deselected) {
            isSelected = resultsSelectionModel.isSelected(delegateModel.modelIndex(index))
        }
    }

    Rectangle{ id: frame
        width: parent.width
        height: parent.height - listSpacing
        anchors.verticalCenter: parent.verticalCenter
        color: isSelected? Qt.darker(highlightColor, 5) : "transparent"
        border.color: isHovered? highlightColor : bgColorNormal
        border.width: borderWidth

        MouseArea{ id: mArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            // wierd workaround for flickable..
            propagateComposedEvents: false
            onReleased: {
                if(!propagateComposedEvents)
                    propagateComposedEvents = true
            }

            onDoubleClicked: (mouse)=> {
                let m = ShotBrowserHelpers.mapIndexesToResultModel([delegateModel.modelIndex(index)])[0].model
                if(m.groupId != helpers.QVariantFromUuidString("087c4ff5-2da0-4e54-afcf-c7914a247fae"))
                    ShotBrowserHelpers.addToCurrent([delegateModel.modelIndex(index)], panelType != "ShotBrowser")
                else
                    ShotBrowserHelpers.addSequencesToCurrentPlaylist([delegateModel.modelIndex(index)])
            }

            onPressed: {
                // required for doubleclick to work
                mouse.accepted = true

                if (mouse.button == Qt.RightButton){
                    if(popupMenu.visible) popupMenu.visible = false
                    else{
                        if(!isSelected) {
                            if(mouse.modifiers == Qt.NoModifier) {
                                ShotBrowserHelpers.selectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                            } else if(mouse.modifiers == Qt.ShiftModifier){
                                ShotBrowserHelpers.shiftSelectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                            } else if(mouse.modifiers == Qt.ControlModifier) {
                                ShotBrowserHelpers.ctrlSelectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                            }
                        }
                        popupMenu.popupDelegateModel = delegateModel
                        let ppos = mapToItem(popupMenu.parent, mouseX, mouseY)
                        popupMenu.x = ppos.x
                        popupMenu.y = ppos.y
                        popupMenu.visible = true
                    }
                }

                else if(mouse.modifiers == Qt.NoModifier) {
                    ShotBrowserHelpers.selectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                } else if(mouse.modifiers == Qt.ShiftModifier){
                    ShotBrowserHelpers.shiftSelectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                } else if(mouse.modifiers == Qt.ControlModifier) {
                    ShotBrowserHelpers.ctrlSelectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                }
            }
        }


        XsPrimaryButton{ id: versionArrowBtn
            width: visible? 20 : 0
            height: parent.height - itemSpacing*4
            anchors.verticalCenter: parent.verticalCenter
            x: (modelDepth * 40)+ itemSpacing*3

            visible: groupingEnabled
            imgSrc: "qrc:/icons/chevron_right.svg"
            imageDiv.rotation: isActive ? 90 : 0
            isActiveViaIndicator: false
            enabled: groupingEnabled && delegateModel.notifyModel.hasChildren(index)
            isActive: groupingEnabled && delegateModel.notifyModel.isExpanded(index)

            onClicked: {
                if(!isActive) {
                    delegateModel.notifyModel.expandRow(index)
                    isActive = delegateModel.notifyModel.isExpanded(index)
                } else {
                    delegateModel.notifyModel.collapseRow(index)
                    isActive = delegateModel.notifyModel.isExpanded(index)
                }
            }
        }

        ColumnLayout{ id: col
            width: parent.width - (versionArrowBtn.x + versionArrowBtn.width) - itemSpacing
            height: parent.height- itemSpacing*2
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: versionArrowBtn.right
            anchors.leftMargin: itemSpacing
            spacing: itemSpacing

            Rectangle{ id: shotTitle
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: itemHeight
                color: bgColorNormal

                XsText{
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignLeft
                    text: nameRole
                    font.pixelSize: XsStyleSheet.fontSize * 1.1
                    font.bold: true
                    leftPadding: panelPadding
                }
                XsText{
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignRight
                    text: clientFilenameRole ? clientFilenameRole : ""
                    font.pixelSize: XsStyleSheet.fontSize * 1.1
                    font.bold: true
                    rightPadding: panelPadding
                    opacity: 0.5
                }
            }

            RowLayout{
                Layout.fillWidth: true
                Layout.preferredHeight: (itemHeight * rowCount) + (spacing * (rowCount-1))
                spacing: itemSpacing
                x: spacing

                property int rowCount: 3

                ShotHistorySection1{ id: sec1
                    Layout.minimumWidth: 154
                    Layout.preferredWidth: 154
                    Layout.fillHeight: true
                    clip: true
                }

                ShotHistorySection2{ id: sec2
                    Layout.fillWidth: true
                    Layout.minimumWidth: 115
                    Layout.fillHeight: true
                    clip: true
                }

                ShotHistorySection3{ id: sec3
                    Layout.minimumWidth: 157
                    Layout.preferredWidth: 157
                    Layout.fillHeight: true
                    clip: true
                }

            }

        }


    }
}