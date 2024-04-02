// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3 //for ColorDialog
import QtGraphicalEffects 1.15 //for RadialGradient
import Qt.labs.qmlmodels 1.0 //for RadialGradient

import xStudioReskin 1.0
import xstudio.qml.bookmarks 1.0
import xstudio.qml.clipboard 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

import MaskTool 1.0

Item {

    id: dialog

    // make a read only binding to the "attrs.grading_tool_active" backend attribute
    property bool gradingToolActive: attrs.grading_tool_active
    onGradingToolActiveChanged: {
        console.log("gradingToolActive", gradingToolActive)
    }

    /* This connects to the backend model data named grading_settings to which
    many of our attributes have been added*/
    XsModuleData {
        id: grading_tool_attrs_data
        modelDataName: "grading_settings"
    }

    GradingAttrs {
        id: attrs
    }
    property alias attrs: attrs
    property alias grading_sliders_model: attrs.grading_sliders_model

    property var copy_buffer: []

    property var grading_bookmark: attrs.grading_bookmark
    onGrading_bookmarkChanged: {
        // console.log("GradingBookmark changed from backend " + attrs.grading_bookmark)
        var index = bookmarkFilterModel.sourceModel.search(
            helpers.QVariantFromUuidString(attrs.grading_bookmark), "uuidRole")
        if (index.valid) {
            bookmarkList.currentIndex = bookmarkFilterModel.mapFromSource(index).row
        }
    }

    XsBookmarkFilterModel {
        id: bookmarkFilterModel
        sourceModel: bookmarkModel
        currentMedia: onScreenMediaUuid // this property is made visible by XsSessionWindow
        showHidden: true
    }

    XsAttributeValue {
        id: __playheadLogicalFrame
        attributeTitle: "Logical Frame"
        model: currentPlayheadData
    }
    XsAttributeValue {
        id: __playheadPositionSeconds
        attributeTitle: "Position Seconds"
        model: currentPlayheadData
    }
    property alias playheadLogicalFrame: __playheadLogicalFrame.value
    property alias playheadPositionSeconds: __playheadPositionSeconds.value

    onVisibleChanged: {
        if (visible) {
            attrs.grading_tool_active = true
        } else {
            attrs.grading_tool_active = false
        }
    }

    FileDialog {
        id: cdl_save_dialog
        title: "Save CDL"
        defaultSuffix: "cdl"
        folder: shortcuts.home
        nameFilters:  [ "CDL files (*.cdl)", "CC files (*.cc)", "CCC files (*.ccc)" ]
        selectExisting: false

        onAccepted: {
            // defaultSuffix doesn't seem to work in the current Qt version used
            var path = fileUrl.toString()
            if (!path.endsWith(".cdl") && !path.endsWith(".cc") && !path.endsWith(".ccc")) {
                path += ".cdl"
            }

            attrs.grading_action = "Save CDL " + path
        }
    }

    function hasActiveGrade() {
        return attrs.grading_bookmark && attrs.grading_bookmark != "00000000-0000-0000-0000-000000000000"
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 3

        ColumnLayout {
            Layout.topMargin: 1
            spacing: 3

            Rectangle {
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                Layout.minimumWidth: 190
                Layout.maximumWidth: 190
                Layout.topMargin: 0
                Layout.bottomMargin: 0
                Layout.leftMargin: 0

                color: "transparent"
                opacity: 1.0
                border.width: 1
                border.color: XsStyleSheet.menuBorderColor
                radius: 2

                RowLayout {
                    anchors.fill: parent
                    Layout.topMargin: 0
                    spacing: 3

                    XsPrimaryButton {
                        text: "List"
                        textDiv.font.bold: true
                        //tooltip: "View the colour corrections for the current media"
                        isActive: attrs.tool_panel == "CC"
                        Layout.minimumWidth: 55
                        Layout.maximumWidth: 55
                        Layout.maximumHeight: 30

                        onClicked: {
                            attrs.tool_panel = "CC"
                        }
                    }

                    XsPrimaryButton {
                        text: "Mask"
                        textDiv.font.bold: true
                        //tooltip: "Enable masking, default mask starts empty"
                        isActive: attrs.tool_panel == "Mask"
                        Layout.maximumWidth: 70
                        Layout.maximumHeight: 30

                        onClicked: {
                            attrs.tool_panel = attrs.tool_panel == "Mask" ? "CC" : "Mask"
                        }
                    }

                    Item {
                        // Spacer item
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }

            MaskDialog {
                id: maskDialog

                enabled: attrs.tool_panel == "Mask"
                visible: attrs.tool_panel == "Mask"

                Layout.minimumWidth: 190
                Layout.maximumWidth: 190
                Layout.minimumHeight: 275
                Layout.maximumHeight: 275
                Layout.topMargin: 0
            }

            Rectangle {
                id: cclistDialog

                enabled: attrs.tool_panel == "CC"
                visible: attrs.tool_panel == "CC"

                Layout.minimumWidth: 190
                Layout.maximumWidth: 190
                Layout.minimumHeight: 275
                Layout.maximumHeight: 275
                Layout.topMargin: 0

                color: "transparent"

                ColumnLayout {

                    ListView {
                        id: bookmarkList

                        Layout.fillHeight: true
                        Layout.maximumWidth: cclistDialog.width

                        implicitWidth: 200
                        implicitHeight: 285

                        model: bookmarkFilterModel
                        orientation: ListView.Vertical

                        ScrollBar.vertical: ScrollBar {
                            id: scrollbar

                            contentItem: Rectangle {
                                implicitWidth: 6
                                color: scrollbar.hovered ? "lightGrey" : "darkGrey"
                            }
                        }

                        onCurrentIndexChanged: {
                            if (currentIndex < 0) {
                                attrs.grading_bookmark = helpers.QUuidToQString("00000000-0000-0000-0000-000000000000")
                            }
                        }

                        onCurrentItemChanged: {
                            if (currentItem) {
                                var backendUuid = helpers.QVariantFromUuidString(attrs.grading_bookmark)
                                var selectedUuid = currentItem.uuidRole

                                if (backendUuid != selectedUuid && selectedUuid) {
                                    attrs.grading_bookmark = helpers.QUuidToQString(selectedUuid)
                                }
                            }
                        }

                        delegate: Item {
                            width: 180
                            height: 25

                            required property int index
                            required property string subjectRole
                            required property string uuidRole

                            readonly property bool is_selected: index == ListView.view.currentIndex

                            XsPrimaryButton {
                                anchors.fill: parent
                                isActive: is_selected
                                text: parent.subjectRole + " " + parent.uuidRole
                                textDiv.elide: Text.ElideRight
                                font.bold: is_selected
                                onClicked: {
                                    parent.ListView.view.currentIndex = parent.index
                                }
                            }
                        }

                        Rectangle {
                            anchors.fill: parent

                            color: "transparent"
                            opacity: 1.0
                            border.width: 1
                            border.color: XsStyleSheet.menuBorderColor
                            radius: 2
                        }
                    }
                }
            }

            Item {
                // Spacer item
                Layout.fillHeight: true
            }
        }

        ColumnLayout {
            Layout.topMargin: 1
            spacing: 3

            Rectangle {
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                Layout.fillWidth: true

                color: "transparent"
                opacity: 1.0
                border.width: 1
                border.color: XsStyleSheet.menuBorderColor
                radius: 2

                RowLayout {
                    anchors.fill: parent
                    Layout.topMargin: 0
                    spacing: 3

                    Repeater {
                        model: attrs.grading_panel_options

                        XsPrimaryButton {
                            text: attrs.grading_panel_options[index]
                            textDiv.font.bold: true
                            //tooltip: "Basic grading controls (restricted to work within a single CDL)"
                            isActive: attrs.grading_panel == text
                            Layout.maximumWidth: 70
                            Layout.maximumHeight: 30
                            onClicked: {
                                attrs.grading_panel = text
                            }
                        }
                    }

                    Item {
                        // Spacer item
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }

            Rectangle {
                Layout.leftMargin: 0
                Layout.bottomMargin: 0
                Layout.fillWidth: true
                Layout.fillHeight: true

                color: "transparent"
                opacity: 1.0
                border.width: 1
                border.color: XsStyleSheet.menuBorderColor
                radius: 2

                visible: attrs.grading_panel == "Basic"

                Column {
                    anchors.fill: parent
                    GradingSliderSimple {
                    }

                }
            }

            Rectangle {
                Layout.leftMargin: 0
                Layout.bottomMargin: 0
                Layout.fillWidth: true
                Layout.fillHeight: true

                color: "transparent"
                opacity: 1.0
                border.width: 1
                border.color: XsStyleSheet.menuBorderColor
                radius: 2

                visible: attrs.grading_panel == "Sliders"

                RowLayout {

                    anchors.topMargin: 10
                    anchors.leftMargin: 10
                    anchors.fill: parent
                    spacing: 15

                    Repeater {
                        model: grading_sliders_model
                        delegate: chooser
                    }

                    DelegateChooser {
                        id: chooser
                        role: "title"
                        DelegateChoice{
                            roleValue: "Saturation"
                            GradingSingleSlider {
                                Layout.fillHeight: true
                                title: model.title
                            }
                        }
                        DelegateChoice{
                            GradingSliderGroup {
                                Layout.fillHeight: true
                                title: model.title
                            }
                        }
                    }

                }
            }

            Rectangle {
                Layout.leftMargin: 0
                Layout.bottomMargin: 0
                Layout.fillWidth: true
                Layout.fillHeight: true

                color: "transparent"
                opacity: 1.0
                border.width: 1
                border.color: XsStyleSheet.menuBorderColor
                radius: 2

                visible: attrs.grading_panel == "Wheels"

                RowLayout {
                    anchors.topMargin: 10
                    anchors.leftMargin: 10
                    anchors.fill: parent
                    spacing: 15

                    Repeater {
                        model: grading_sliders_model
                        delegate: slider_chooser
                    }

                    DelegateChooser {
                        id: slider_chooser
                        role: "title"
                        DelegateChoice{
                            roleValue: "Saturation"
                            GradingSingleSlider {
                                Layout.fillHeight: true
                                title: model.title
                                width: 160
                            }
                        }
                        DelegateChoice{
                            GradingWheel {
                                Layout.fillHeight: true
                                title: model.title
                                width: 160
                            }
                        }
                    }

                }
            }

            Rectangle {
                color: "transparent"
                opacity: 1.0
                border.width: 1
                border.color: XsStyleSheet.menuBorderColor
                radius: 2

                Layout.topMargin: 1
                Layout.minimumHeight: 25
                Layout.maximumHeight: 25
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent
                    layoutDirection: Qt.RightToLeft

                    XsPrimaryButton {
                        Layout.maximumWidth: 70
                        Layout.maximumHeight: 25
                        text: "Bypass All"
                        //tooltip: "Bypass all CDLs or not"
                        isActive: attrs.drawing_bypass

                        onClicked: {
                            attrs.drawing_bypass = !attrs.drawing_bypass
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 90
                        Layout.maximumHeight: 25
                        text: "Track grade"
                        //tooltip: "Auto select grade depending on currently shown frame"
                        isActive: attrs.grading_tracking

                        onClicked: {
                            attrs.grading_tracking = !attrs.grading_tracking
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 40
                        Layout.maximumHeight: 25
                        text: "+"
                        //tooltip: "Add a color correction"

                        onClicked: {
                            attrs.grading_action = "Add CC"
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 40
                        Layout.maximumHeight: 25
                        text: "-"
                        //tooltip: "Remove the currently selected color correction"
                        enabled: bookmarkList.count > 0

                        onClicked: {
                            attrs.grading_action = "Remove CC"
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 40
                        Layout.maximumHeight: 25
                        text: "Copy"
                        //tooltip: "Copy current colour correction"
                        enabled: hasActiveGrade()
                        onClicked: {
                            var attr_values = []
                            for (var i = 0; i < grading_sliders_model.length; ++i) {
                                attr_values.push(grading_sliders_model.get(grading_sliders_model.index(i,0),"value"))
                            }
                            dialog.copy_buffer = attr_values
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 40
                        Layout.maximumHeight: 25
                        text: "Paste"
                        //tooltip: "Paste colour correction"
                        enabled: dialog.copy_buffer.length == grading_sliders_model.length
                        onClicked: {
                            for (var i = 0; i < grading_sliders_model.length; ++i) {
                                grading_sliders_model.set(
                                    grading_sliders_model.index(i,0),
                                    dialog.copy_buffer[i],
                                    "value"
                                    )
                            }
                        }
                    }

                    Item {
                        // Spacer item
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 40
                        Layout.maximumHeight: 25
                        text: "Out"
                        //tooltip: "Grade ends at this frame"
                        enabled: hasActiveGrade()

                        onClicked: {
                            attrs.grade_out = playheadLogicalFrame
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 40
                        Layout.maximumHeight: 25
                        text: "In"
                        //tooltip: "Grade starts at this frame"
                        enabled: hasActiveGrade()

                        onClicked: {
                            attrs.grade_in = playheadLogicalFrame
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 90
                        Layout.maximumHeight: 25
                        text: "Full clip"
                        //tooltip: "Grade applies on the full duration of the media"
                        enabled: hasActiveGrade()

                        onClicked: {
                            attrs.grade_in = -1
                            attrs.grade_out = -1
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 90
                        Layout.maximumHeight: 25
                        text: "Single frame"
                        //tooltip: "Grade applies on the current frame only"
                        enabled: hasActiveGrade()

                        onClicked: {
                            console.log("Set bookmark in/out to " + playheadLogicalFrame)
                            attrs.grade_in = playheadLogicalFrame
                            attrs.grade_out = playheadLogicalFrame
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 50
                        Layout.maximumHeight: 25
                        text: "Bypass"
                        //tooltip: "Apply CDL or not"
                        isActive: !attrs.grade_active
                        enabled: hasActiveGrade()

                        onClicked: {
                            attrs.grade_active = !attrs.grade_active
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 120
                        Layout.maximumHeight: 25
                        text: hasActiveGrade() ? attrs.colour_space : attrs.working_space
                        //tooltip: "Toggle process colour space"
                        enabled: hasActiveGrade() && attrs.media_colour_managed

                        onClicked: {
                            attrs.colour_space = attrs.colour_space == "scene_linear" ? "compositing_log" : "scene_linear"
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 58
                        Layout.maximumHeight: 25
                        text: "Reset All"
                        //tooltip: "Reset CDL parameters to default"
                        enabled: hasActiveGrade()

                        onClicked: {
                            attrs.grading_action = "Clear"
                        }
                    }

                    XsPrimaryButton {
                        Layout.maximumWidth: 80
                        Layout.maximumHeight: 25
                        text: "Save CDL ..."
                        //tooltip: "Save CDL to disk as a .cdl, .cc or .ccc"
                        enabled: hasActiveGrade()

                        onClicked: {
                            cdl_save_dialog.open()
                        }
                    }

                    XsPrimaryButton {
                        Layout.minimumWidth: 110
                        Layout.maximumWidth: 120
                        Layout.maximumHeight: 25
                        text: "Copy Nuke Node"
                        //tooltip: "Copy CDL as a Nuke OCIOCDLTransform node to the clipboard - paste into Nuke node graph with CTRL+V"
                        enabled: hasActiveGrade()

                        Clipboard {
                            id: clipboard
                        }

                        onClicked: {

                            var slope = attrs.getAttrValue("Slope")
                            var offset = attrs.getAttrValue("Offset")
                            var power = attrs.getAttrValue("Power")
                            var sat = attrs.getAttrValue("Saturation")

                            var cdl_node = "OCIOCDLTransform {\n"
                            if (attrs.colour_space != "scene_linear") {
                                cdl_node += "  working_space " + attrs.colour_space + "\n"
                            }
                            cdl_node += "  slope { "
                            cdl_node += (slope[0] * slope[3]) + " "
                            cdl_node += (slope[1] * slope[3]) + " "
                            cdl_node += (slope[2] * slope[3]) + " "
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
                    }

                }
            }
        }

    }
}