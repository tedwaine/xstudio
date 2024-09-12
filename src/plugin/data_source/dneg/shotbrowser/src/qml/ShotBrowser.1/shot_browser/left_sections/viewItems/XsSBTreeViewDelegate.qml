// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudio 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

MouseArea {
    id: dragArea

    height: treeNode.height
    anchors.left: parent.left
    anchors.right: parent.right
    hoverEnabled: true

    property real treeItemHeight: btnHeight - 4
    property real treeItemSpacing: 1

    property bool isHovered: nodeDivMArea.containsMouse
    property bool isExpanded: expandedModel.isSelected(delegateModel.modelIndex(index))
    property bool isSelected: selectionModel.isSelected(delegateModel.modelIndex(index))

    property bool isParent: typeRole == "Sequence" && childCountRole
    property bool isPopulated: false

    property var delegateModel: null
    property var selectionModel: null
    property var expandedModel: null

    onClicked: (mouse) => {
        if(mouse.modifiers == Qt.NoModifier) {
            ShotBrowserHelpers.selectItem(selectionModel, delegateModel.modelIndex(index))
        } else if(mouse.modifiers == Qt.ShiftModifier){
            ShotBrowserHelpers.shiftSelectItem(selectionModel, delegateModel.modelIndex(index))
        } else if(mouse.modifiers == Qt.ControlModifier) {
            ShotBrowserHelpers.ctrlSelectItem(selectionModel, delegateModel.modelIndex(index))
        }
    }

    onIsSelectedChanged: {
        if(isSelected) {
            sequenceTreeView.jumpTo(this)
        }
    }

    Connections {
        target: expandedModel
        function onSelectionChanged(selected, deselected) {
            isExpanded = expandedModel.isSelected(delegateModel.modelIndex(index))
        }
    }

    Connections {
        target: selectionModel
        function onSelectionChanged(selected, deselected) {
            isSelected = selectionModel.isSelected(delegateModel.modelIndex(index))
        }
    }

    onIsExpandedChanged: {
        if(isExpanded) {
            if(!isPopulated) {
                createTreeNode(childView)
                isPopulated = true
            }
        }
    }

    Component.onCompleted: {
        if(isExpanded && !isPopulated) {
            createTreeNode(childView)
            isPopulated = true
        }
    }

    Rectangle {
        id: treeNode
        color: "transparent"
        height: treeItemHeight + (isExpanded ? childView.height : 0)
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        MouseArea{ id: nodeDivMArea
            anchors.fill: nodeDiv
            hoverEnabled: true
            propagateComposedEvents: true
            onClicked: (mouse)=>{
                mouse.accepted = false
            }
        }
        Rectangle{ id: nodeDiv
            color: isSelected ? Qt.darker(palette.highlight, 2) : "transparent"
            border.width: 1
            border.color: isHovered? palette.highlight : "transparent"

            height: treeItemHeight - treeItemSpacing
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }

            RowLayout{
                spacing: buttonSpacing
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    verticalCenter: parent.verticalCenter
                }
                height: btnHeight

                XsSecondaryButton{ id: expandButton

                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.preferredWidth: height/1.2
                    Layout.preferredHeight: parent.height //- panelPadding * 2

                    imgSrc: "qrc:/icons/chevron_right.svg"
                    isActive: isExpanded
                    enabled: isParent
                    opacity: enabled? 1 : 0.5

                    // scale: rotation==0 || rotation==90? 1:0.85
                    imageDiv.rotation: (isExpanded)? 90:0

                    onClicked: expandedModel.select(delegateModel.modelIndex(index), ItemSelectionModel.Toggle)
                }
                // Item{
                //     Layout.preferredWidth: 1
                //     Layout.preferredHeight: parent.height
                // }

                XsText{
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                    opacity: ["na", "del", "omt", "omtnto", "omtnwd"].includes(statusRole) ? 0.5 : 1.0

                    color: "hld" == statusRole ? "red" : palette.text

                    text: nameRole
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: XsStyleSheet.fontSize*1.2
                    elide: Text.ElideRight
                    leftPadding: 2
                }
                XsText{
                    Layout.preferredHeight: parent.height
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    opacity: 0.5

                    text: statusRole  ? statusRole : ""
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: XsStyleSheet.fontSize*1.2
                    elide: Text.ElideRight
                    rightPadding: 8
                }
            }
        }

        Item {
            id: childView
            anchors {
                left: parent.left
                right: parent.right
                top: nodeDiv.bottom

                leftMargin: panelPadding*4
                topMargin: treeItemSpacing
            }
            visible: isExpanded
            height: childrenRect.height
        }
    }

    function createTreeNode(parentNode) {
        let newnode = Qt.createComponent("XsSBTreeView.qml").createObject(
            parentNode,
            {
                width: parentNode.width,
                treeSequenceModel: delegateModel.model,
                treeSequenceSelectionModel: selectionModel,
                treeSequenceExpandedModel: expandedModel,
                treeRootIndex: delegateModel.modelIndex(index)
            }
        )
        if(newnode == null) {
            console.log("Error creating object");
        } else {
            newnode.width = Qt.binding(function() { return childView.width })
        }
    }

}

