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

import xStudioReskin 1.0
import xstudio.qml.models 1.0

Item{ id: toolActionsDiv

    height: toolActionUndoRedo.height + toolActionCopyPasteClear.height + displayBtns.height

    XsAttributeValue {
        id: __action_attr
        attributeTitle: "action_attribute"
        model: annotations_model_data
    }

    property alias action_attribute: __action_attr.value

    ListView{ id: toolActionUndoRedo

        width: parent.width - framePadding*2
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

        width: parent.width - framePadding*2
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

    Rectangle{ id: displayAnnotations
        width: parent.width - framePadding
        height: buttonHeight;
        color: "transparent";
        anchors.top: toolActionCopyPasteClear.bottom
        anchors.topMargin: colSpacing
        anchors.horizontalCenter: parent.horizontalCenter

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

    RowLayout{ id: displayBtns
        x: framePadding
        width: parent.width- x*2 - spacing
        height: XsStyleSheet.primaryButtonStdHeight
        spacing: 1

        anchors.top: displayAnnotations.bottom
        anchors.topMargin: itemSpacing/2

        XsPrimaryButton{

            id: annotationBtn
            Layout.preferredWidth: parent.width/2
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            text: "A"
            textDiv.font.pixelSize: XsStyleSheet.fontSize*1.2
            onClicked:{
                if(annotationMenu.visible) annotationMenu.visible = false
                else{
                    annotationMenu.x = displayBtns.x + x //+ width
                    annotationMenu.y = displayBtns.y + y + height
                    annotationMenu.visible = true
                }
            }
        }
    }

    XsPopupMenu {
        id: annotationMenu
        visible: false
        menu_model_name: "annotationMenu"+toolActionsDiv
    }

    XsMenuModelItem {
        text: "Always"
        enabled: annotationBtn.text !== "A"
        menuPath: ""
        menuItemPosition: 1
        menuModelName: annotationMenu.menu_model_name
        onActivated: {
            annotationBtn.text = "A"
            display_mode = "Always"
        }
    }
    XsMenuModelItem {
        text: "Never"
        enabled: annotationBtn.text !== "N"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: annotationMenu.menu_model_name
        onActivated: {
            annotationBtn.text = "N"
            display_mode = "With Drawing Tools"
        }
    }
    XsMenuModelItem {
        text: "When Paused"
        enabled: annotationBtn.text !== "P"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: annotationMenu.menu_model_name
        onActivated: {
            annotationBtn.text = "P"
            display_mode = "Only When Paused"
        }
    }

}

