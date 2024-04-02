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
import xstudio.qml.module 1.0

Item{

    ListView{ id: toolActionUndoRedo

        width: parent.width - framePadding*2
        height: buttonHeight
        x: framePadding
        y: framePadding + spacing/2

        spacing: itemSpacing
        interactive: false
        orientation: ListView.Horizontal

        model: ListModel{
            id: modelUndoRedo
            ListElement{
                action: "Undo"
            }
            ListElement{
                action: "Redo"
            }
        }

        delegate: XsPrimaryButton{
            text: model.action
            imgSrc: ""
            width: toolActionUndoRedo.width/modelUndoRedo.count - toolActionUndoRedo.spacing
            height: buttonHeight

            onClicked: {
                anno_tool_backend_settings.action_attribute = text
            }
        }
    }

    ListView{ id: toolActionCopyPasteClear

        width: parent.width - framePadding*2
        height: buttonHeight
        x: framePadding //+ spacing/2
        y: toolActionUndoRedo.y + toolActionUndoRedo.height + spacing

        spacing: itemSpacing
        interactive: false
        orientation: ListView.Horizontal

        model:
        ListModel{
            id: modelCopyPasteClear
            ListElement{ action: "Clear" }
        }
        
        delegate:
        XsPrimaryButton{
            text: model.action
            width: toolActionCopyPasteClear.width/modelCopyPasteClear.count - toolActionCopyPasteClear.spacing
            height: buttonHeight
            enabled: text == "Clear"
            
            onClicked: {
                anno_tool_backend_settings.action_attribute = text
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
            text: "Display Annotations"
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

    XsModuleAttributes {
        // this lets us get at the combo_box_options for the 'Display Mode' attr
        id: annotations_tool_draw_mode_options
        attributesGroupNames: "annotations_tool_draw_mode_0"
        roleName: "combo_box_options"
    }

    XsModuleAttributes {
        // this lets us get at the value for the 'Display Mode' attr
        id: annotations_tool_draw_mode
        attributesGroupNames: "annotations_tool_draw_mode_0"
    }

    XsComboBox {

        id: dropdownAnnotations

        property var displayModeOptions: annotations_tool_draw_mode_options.display_mode ? annotations_tool_draw_mode_options.display_mode : []
        property var displayModeValue: annotations_tool_draw_mode.display_mode ? annotations_tool_draw_mode.display_mode : ""

        model: displayModeOptions
        width: parent.width/1.3;
        height: buttonHeight
        anchors.top: displayAnnotations.bottom
        // anchors.topMargin: itemSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        onCurrentTextChanged: {
            if (currentText != displayModeValue && annotations_tool_draw_mode.display_mode != undefined) {
                annotations_tool_draw_mode.display_mode = currentText
            }
        }
        onDisplayModeValueChanged: {

            if (displayModeOptions.indexOf(displayModeValue) != -1) {
                currentIndex = displayModeOptions.indexOf(displayModeValue)
            }
        }

    }

}

