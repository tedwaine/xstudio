// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

import xStudio 1.0
import xstudio.qml.models 1.0
import xstudio.qml.clipboard 1.0

Item{ id: leftSec
    property alias bookmarkList: listDiv.bookmarkList
    property var copy_buffer: []

    Clipboard{
        id: clipboard
    }

    XsGradientRectangle{
        anchors.fill: parent
    }

    ColumnLayout { id: leftView
        anchors.fill: parent
        anchors.margins: panelPadding
        spacing: panelPadding

        L1Header{
            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight
        }
        L2LayerList{ id: listDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    XsPopupMenu {
        id: moreMenu
        visible: false
        menu_model_name: "moreMenu"+leftSec
    }

    XsMenuModelItem {
        menuItemType: "radiogroup"
        choices: attrs.media_colour_managed ? ["scene_linear", "compositing_log"] : ["raw"]
        
        property string currentColorSpace: attrs.media_colour_managed ? attrs.colour_space : "raw"
        
        currentChoice: currentColorSpace
        onCurrentChoiceChanged: {
            if (currentChoice != "raw")
                attrs.colour_space = currentChoice
        }

        enabled: hasActiveGrade() && attrs.media_colour_managed
        text: ""
        menuPath: "Color Space"
        menuItemPosition: 1
        menuModelName: moreMenu.menu_model_name
        onActivated: {
            cdl_save_dialog.open()
        }
    }
    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: moreMenu.menu_model_name
    }

    XsMenuModelItem {
        text: "Rename..."
        enabled: false
        menuPath: ""
        menuItemPosition: 3
        menuModelName: moreMenu.menu_model_name
        onActivated: {}
    }
    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 4
        menuModelName: moreMenu.menu_model_name
    }
    XsMenuModelItem {
        text: "Copy"
        enabled: hasActiveGrade()
        menuPath: ""
        menuItemPosition: 5
        menuModelName: moreMenu.menu_model_name
        onActivated: {
            copyFunction()
        }
    }
    XsMenuModelItem {
        text: "Paste"
        enabled: copy_buffer.length == (grading_sliders_model.length + grading_wheels_model.length)
        menuPath: ""
        menuItemPosition: 6
        menuModelName: moreMenu.menu_model_name
        onActivated: {
            pasteFunction();
        }
    }
    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 7
        menuModelName: moreMenu.menu_model_name
    }
    XsMenuModelItem {
        text: "Copy Nuke Node"
        menuPath: ""
        menuItemPosition: 8
        menuModelName: moreMenu.menu_model_name
        onActivated: {
            copyNukeNode();
        }
    }
    XsMenuModelItem {
        text: "Save CDL..."
        menuPath: ""
        menuItemPosition: 9
        menuModelName: moreMenu.menu_model_name
        onActivated: {
            cdl_save_dialog.open()
        }
    }

    function copyFunction() {
        var attr_values = []
        for (var i = 0; i < grading_sliders_model.length; ++i) {
            attr_values.push(grading_sliders_model.get(grading_sliders_model.index(i,0),"value"))
        }
        for (var i = 0; i < grading_wheels_model.length; ++i) {
            attr_values.push(grading_wheels_model.get(grading_wheels_model.index(i,0),"value"))
        }
        copy_buffer = attr_values
    }

    function pasteFunction() {
        for (var i = 0; i < grading_sliders_model.length; ++i) {
            grading_sliders_model.set(
                grading_sliders_model.index(i,0),
                copy_buffer[i],
                "value"
            )
        }
        for (var i = 0; i < grading_wheels_model.length; ++i) {
            grading_wheels_model.set(
                grading_wheels_model.index(i,0),
                copy_buffer[grading_sliders_model.length + i],
                "value"
            )
        }
    }

    function copyNukeNode() {

        // TODO: ColSci
        // Use Grade node instead of OCIOCDLTransform to handle contrast?

        var offset = attrs.getAttrValue("Offset")
        var power = attrs.getAttrValue("Power")
        var slope = attrs.getAttrValue("Slope")
        var sat = attrs.getAttrValue("Saturation")
        var exp = attrs.getAttrValue("Exposure")
        var cont = attrs.getAttrValue("Contrast")

        var cdl_node = "OCIOCDLTransform {\n"
        if (attrs.colour_space != "scene_linear") {
            cdl_node += "  working_space " + attrs.colour_space + "\n"
        }
        cdl_node += "  slope { "
        cdl_node += (slope[0] * slope[3] * Math.pow(2.0, exp)) + " "
        cdl_node += (slope[1] * slope[3] * Math.pow(2.0, exp)) + " "
        cdl_node += (slope[2] * slope[3] * Math.pow(2.0, exp)) + " "
        cdl_node += "}\n"
        cdl_node += "  offset { "
        cdl_node += (offset[0] + offset[3]) + " "
        cdl_node += (offset[1] + offset[3]) + " "
        cdl_node += (offset[2] + offset[3]) + " "
        cdl_node += "}\n"
        cdl_node += "  power { "
        cdl_node += (power[0] * power[3]) + " "
        cdl_node += (power[1] * power[3]) + " "
        cdl_node += (power[2] * power[3]) + " "
        cdl_node += "}\n"
        cdl_node += "  saturation " + sat + "\n"
        cdl_node += "}"

        clipboard.text = cdl_node
    }

    FileDialog {
        id: cdl_save_dialog
        title: "Save CDL"
        defaultSuffix: "cdl"
        folder: shortcuts.home
        nameFilters:  [ "CDL files (*.cdl)", "CC files (*.cc)", "CCC files (*.ccc)" ]
        selectExisting: false

        // TODO: ColSci
        // Add warning if contrast is used?

        onAccepted: {
            // defaultSuffix doesn't seem to work in the current Qt version used
            var path = fileUrl.toString()
            if (!path.endsWith(".cdl") && !path.endsWith(".cc") && !path.endsWith(".ccc")) {
                path += ".cdl"
            }

            attrs.grading_action = "Save CDL " + path
        }
    }


}
