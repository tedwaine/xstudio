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

    height: cBox.y + cBox.height + 5

    XsAttributeValue {
        id: __action_attr
        attributeTitle: "action_attribute"
        model: annotations_model_data
    }

    property alias action_attribute: __action_attr.value

    ListView{ id: toolActionUndoRedo

        width: parent.width - framePadding*2
        height: XsStyleSheet.primaryButtonStdHeight
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
            ListElement{
                action: "Clear"
                icon: "qrc:///anno_icons/delete.svg"
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


    Rectangle{ id: displayAnnotations
        width: parent.width - framePadding
        height: buttonHeight/2
        color: "transparent";
        anchors.top: toolActionUndoRedo.bottom
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

    XsComboBox { id: cBox
        x: framePadding
        width: parent.width- x*3
        height: XsStyleSheet.primaryButtonStdHeight
        anchors.top: displayAnnotations.bottom
        anchors.topMargin: itemSpacing

        model: ["Always", "Never", "When Paused"]
        currentIndex: 0
        framePadding: 2
        fontSize: XsStyleSheet.fontSize - 1

        onActivated: (aindex) => {
            if(textAt(aindex) == "Always") {
                display_mode = "Always"
                cBox.displayText = "Always"
            }
            else if(textAt(aindex) == "Never") {
                display_mode = "With Drawing Tools"
                cBox.displayText = "Never"
            }
            else if(textAt(aindex) == "When Paused") {
                display_mode = "Only When Paused"
                cBox.displayText = "Paused"
            }
        }
    }

}

