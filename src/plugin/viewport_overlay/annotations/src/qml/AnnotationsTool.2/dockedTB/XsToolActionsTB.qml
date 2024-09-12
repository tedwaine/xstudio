// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3
import QtGraphicalEffects 1.15

import xStudio 1.0
import xstudio.qml.models 1.0

Item{ id: toolActionsDiv

    // height: toolActionUndoRedo.height + toolActionCopyPasteClear.height //+ displayBtns.height

    XsAttributeValue {
        id: __action_attr
        attributeTitle: "action_attribute"
        model: annotations_model_data
    }

    property alias action_attribute: __action_attr.value

    ListView{ id: toolActionUndoRedo

        width: parent.height - framePadding*2
        height: XsStyleSheet.primaryButtonStdHeight //buttonHeight
        x: framePadding
        y: framePadding + spacing/2

        spacing: itemSpacing
        interactive: false
        orientation: ListView.Horizontal

        model: ListModel{
            id: modelUndoRedo
            ListElement{
                action: "Undo"
                icon: "qrc:///anno_icons/undo.svg"
            }
            ListElement{
                action: "Redo"
                icon: "qrc:///anno_icons/redo.svg"
            }
        }

        delegate: XsPrimaryButton{
            text: "" //model.action
            imgSrc: model.icon //""
            width: toolActionUndoRedo.width/modelUndoRedo.count - toolActionUndoRedo.spacing
            height: toolActionUndoRedo.height //buttonHeight

            onClicked: {
                action_attribute = model.action
            }
        }
    }

    ListView{ id: toolActionCopyPasteClear

        width: parent.height - framePadding*2
        height: XsStyleSheet.primaryButtonStdHeight //buttonHeight
        x: framePadding //+ spacing/2
        y: toolActionUndoRedo.y + toolActionUndoRedo.height + spacing

        spacing: itemSpacing
        interactive: false
        orientation: ListView.Horizontal

        model:
        ListModel{
            id: modelCopyPasteClear
            ListElement{
                action: "Clear"
                icon: "qrc:///anno_icons/delete.svg"
            }
        }

        delegate:
        XsPrimaryButton{
            text: "" //model.action
            imgSrc: model.icon //""
            width: toolActionCopyPasteClear.width/modelCopyPasteClear.count - toolActionCopyPasteClear.spacing
            height: toolActionCopyPasteClear.height
            onClicked: {
                action_attribute = model.action
            }

        }
    }



    // Sigh - hooking up the draw mode backen attr to the combo box here
    // is horrible! Need something better than this!
    XsModuleData {
        id: annotations_tool_draw_mode_model
        modelDataName: "annotations_tool_draw_mode"
    }

    XsAttributeValue {
        id: __display_mode
        attributeTitle: "Display Mode"
        model: annotations_tool_draw_mode_model
    }
    property alias display_mode: __display_mode.value

    ColumnLayout{ id: displayBtns
        // x: framePadding
        width: parent.height - framePadding*2
        height: XsStyleSheet.primaryButtonStdHeight*2 + spacing
        spacing: 0

        anchors.left: toolActionCopyPasteClear.right
        // anchors.leftMargin: colSpacing
        anchors.verticalCenter: parent.verticalCenter

        // anchors.top: displayAnnotations.bottom
        // anchors.topMargin: itemSpacing/2
        // anchors.horizontalCenter: displayAnnotations.horizontalCenter

        Rectangle{ id: displayAnnotations

            Layout.fillWidth: true
            Layout.preferredWidth: parent.width - framePadding*2
            Layout.preferredHeight: buttonHeight
            Layout.alignment: Qt.AlignHCenter

            color: "transparent";

            Text{
                text: "Display"
                font.pixelSize: fontSize
                font.family: fontFamily
                color: toolInactiveTextColor
                width: parent.width
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: framePadding
            }
        }
        XsComboBox {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width - framePadding*2
            Layout.preferredHeight: XsStyleSheet.primaryButtonStdHeight
            Layout.alignment: Qt.AlignHCenter
            model: ["Always", "Never", "When Paused"]
            currentIndex: 0
            framePadding: 2
            fontSize: XsStyleSheet.fontSize - 1

            onActivated: (aindex) => {
                if(textAt(aindex) == "Always") {
                    display_mode = "Always"
                    displayText = "Always"
                }
                else if(textAt(aindex) == "Never") {
                    display_mode = "With Drawing Tools"
                    displayText = "Never"
                }
                else if(textAt(aindex) == "When Paused") {
                    display_mode = "Only When Paused"
                    displayText = "Paused"
                }
            }
        }
    }


}

