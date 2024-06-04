// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QuickFuture 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

Item{ id: thisItem

    property bool isActive: false
    property bool isSelected: resultsSelectionModel.isSelected(delegateModel.modelIndex(index))

    property real borderWidth: 1

    property real listSpacing: panelPadding
    property real itemSpacing: 1
    property real itemHeight: XsStyleSheet.widgetStdHeight

    property real textSize: XsStyleSheet.fontSize

    property color hintColor: XsStyleSheet.hintColor
    property color highlightColor: palette.highlight
    property color bgColorNormal: XsStyleSheet.widgetBgNormalColor

    property var delegateModel: null
    property var popupMenu: null

    property bool isHovered: mArea.containsMouse ||
        sec1.isHovered ||
        sec2.isHovered

    required property string thumbRole
    required property string noteTypeRole
    required property string createdByRole

    required property string subjectRole
    required property string contentRole
    required property string versionNameRole
    required property string artistRole

    required property var addressingRole
    required property var createdDateRole
    required property int index

    Connections {
        target: resultsSelectionModel
        function onSelectionChanged(selected, deselected) {
            isSelected = resultsSelectionModel.isSelected(delegateModel.modelIndex(index))
        }
    }

    Rectangle{ id: frame
        width: parent.width
        height: parent.height - listSpacing
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

            onPressed: (mouse)=>{
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
                } else if(mouse.modifiers == Qt.NoModifier) {
                    ShotBrowserHelpers.selectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                } else if(mouse.modifiers == Qt.ShiftModifier){
                    ShotBrowserHelpers.shiftSelectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                } else if(mouse.modifiers == Qt.ControlModifier) {
                    ShotBrowserHelpers.ctrlSelectItem(resultsSelectionModel, delegateModel.modelIndex(index))
                }
            }

            onDoubleClicked: (mouse)=> {
                // need to know context, Which panel am I in.
                ShotBrowserHelpers.addToCurrent([delegateModel.modelIndex(index)], panelType != "ShotBrowser")
            }
        }

        RowLayout{
            anchors.fill: parent
            anchors.margins: borderWidth
            spacing: 1

            NotesHistorySection1{ id: sec1
                Layout.preferredWidth: 160
                Layout.fillHeight: true
            }
            NotesHistorySection2{ id: sec2
                Layout.fillWidth: true
                Layout.minimumWidth: parent.width/2
                Layout.fillHeight: true
            }

        }


    }
}