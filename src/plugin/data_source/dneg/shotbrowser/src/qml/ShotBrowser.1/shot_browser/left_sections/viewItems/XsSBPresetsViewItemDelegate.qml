// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

XsPrimaryButton{ id: thisItem
    height: btnHeight

    isActive: isSelected //#TODO
    isActiveIndicatorAtLeft: true
    bgDiv.height: btnHeight - childNodeSpacing

    property real childNodeSpacing: 1

    property var delegateModel: null
    property var selectionModel: null
    property string groupName: ""

    opacity: hiddenRole ? 0.5 : 1.0

    property bool isSelected: selectionModel.isSelected(presetModelIndex())
    property bool isModified: updateRole != undefined ? updateRole : false
    property bool isRunning: queryRunning && presetModelIndex() == currentPresetIndex

    Connections {
        target: selectionModel
        function onSelectionChanged(selected, deselected) {
            isSelected = selectionModel.isSelected(presetModelIndex())
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: (mouse) => {
            if(mouse.modifiers == Qt.NoModifier) {
                resultViewTitle = groupName+" : "+nameRole
                selectionModel.select(presetModelIndex(), ItemSelectionModel.ClearAndSelect)
                activatePreset(presetModelIndex())
            } else if(mouse.modifiers == Qt.ShiftModifier){
                ShotBrowserHelpers.shiftSelectItem(selectionModel, presetModelIndex())
            } else if(mouse.modifiers == Qt.ControlModifier) {
                ShotBrowserHelpers.ctrlSelectItem(selectionModel, presetModelIndex())
            }

            if (mouse.button == Qt.RightButton) {
                showPresetMenu(mouseX+btnHeight, 0) //btnHeight - is the spacing at left for Presets apart from Groups
            }
        }
    }

    function filterModelIndex() {
        return delegateModel.modelIndex(index)
    }

    function presetModelIndex() {
        try {
            return delegateModel.notifyModel.mapToSource(filterModelIndex())
        } catch (err) {
            return ShotBrowserEngine.presetsModel.index(-1,-1)
        }
    }

    function entityType() {
        let i = presetModelIndex().parent.parent
        return i.model.get(i,"entityRole")
    }


    Rectangle{ id: selectedBgDiv
        anchors.fill: parent
        color: isSelected? Qt.darker(palette.highlight, 2): "transparent"
        // opacity: 0.6
    }
    Rectangle{ id: activeIndicatorDiv
        anchors.bottom: parent.bottom
        width: borderWidth*9
        height: parent.height
        color: isActive? bgColorPressed : "transparent"
    }
    RowLayout{
        anchors.fill: parent
        spacing: presetItemSpacing

        XsText{ id: nameDiv
            Layout.fillWidth: true
            Layout.minimumWidth: 50
            Layout.preferredWidth: 100
            Layout.fillHeight: true

            text: isModified? nameRole+"*" : nameRole
            font.weight: isSelected? Font.Bold : Font.Normal
            horizontalAlignment: Text.AlignLeft
            leftPadding: busyIndicator.width //25
            elide: Text.ElideRight
        }

        XsSecondaryButton{ id: editBtn
            Layout.topMargin: presetItemSpacing
            Layout.bottomMargin: presetItemSpacing
            Layout.preferredWidth: height
            Layout.fillHeight: true

            visible: thisItem.hovered || editBtn.isActive

            imgSrc: "qrc:///shotbrowser_icons/edit.svg"
            isActive: presetEditPopup.presetIndex == presetModelIndex() && presetEditPopup.visible
            scale: 0.95

            onClicked: {
                openEditPopup()
            }
        }

        XsSecondaryButton{ id: moreBtn
            Layout.topMargin: presetItemSpacing
            Layout.bottomMargin: presetItemSpacing
            Layout.preferredWidth: height
            Layout.fillHeight: true

            visible: thisItem.hovered || moreBtn.isActive || editBtn.isActive

            imgSrc: "qrc:/icons/more_vert.svg"
            scale: 0.95
            isActive: presetMenu.visible && presetMenu.presetModelIndex == presetModelIndex()
            onClicked:{
                if(presetMenu.visible) {
                    presetMenu.visible = false
                }
                else{
                    showPresetMenu(0,0)
                }
            }
        }

        XsSecondaryButton{ id: favBtn
            Layout.topMargin: presetItemSpacing
            Layout.bottomMargin: presetItemSpacing
            Layout.preferredWidth: height
            Layout.fillHeight: true

            visible: thisItem.hovered || favouriteRole

            showHoverOnActive: favouriteRole && !thisItem.hovered
            isColoured: favouriteRole && thisItem.hovered
            imgSrc: "qrc:///shotbrowser_icons/favorite.svg"
            // isActive: favouriteRole
            scale: 0.95
            onClicked: favouriteRole = !favouriteRole
        }

    }


    function openEditPopup(){

        if( presetEditPopup.visible == true && presetEditPopup.entityName == nameRole) {
            presetEditPopup.visible = false
            return
        }

        presetEditPopup.title = "Edit '"+nameRole+"' Preset"
        presetEditPopup.entityName = nameRole
        presetEditPopup.entityCategory = "Preset"

        if(!presetEditPopup.visible){
            presetEditPopup.x = appWindow.x + appWindow.width/3
            presetEditPopup.y = appWindow.y + appWindow.height/4
        }

        presetEditPopup.visible = true
        presetEditPopup.presetIndex = presetModelIndex()
        presetEditPopup.entityType = entityType()
    }

    function showPresetMenu(xpos, ypos){

        presetMenu.presetModelIndex = presetModelIndex()
        presetMenu.filterModelIndex = filterModelIndex()

        let p = mapToItem(presetMenu.parent, moreBtn.width, 0)

        presetMenu.x = xpos==0? (p.x + thisItem.width - btnHeight) : xpos
        presetMenu.y = xpos==0 && ypos==0 ? p.y : ypos==0? (p.y + btnHeight/2) : ypos
        presetMenu.visible = true
    }

    XsBusyIndicator{ id: busyIndicator
        x: nameDiv.x + 4
        width: height
        height: parent.height
        running: visible
        visible: isRunning
        scale: 0.5
    }

}

