// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14

import xStudioReskin 1.0
import Grading 2.0

import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.bookmarks 1.0

Rectangle{
    color: "transparent"

    property real maskBtnWidth: 100
    property string activeButton: "Polygon"
    property real itemHeight: XsStyleSheet.widgetStdHeight

    ListModel{ id: mask1ToolsModel
        ListElement{
            toolName: "Polygon"
            toolImg: "qrc:/grading_icons/hexagon.svg"
        }
        ListElement{
            toolName: "Ellipse"
            toolImg: "qrc:/icons/radio_button_unchecked.svg"
        }
    }
    ListModel{ id: mask2ToolsModel

        ListElement{
            toolName: "Dodge"
            toolImg: "qrc:/grading_icons/brightness_low.svg"
        }
        ListElement{
            toolName: "Burn"
            toolImg: "qrc:/grading_icons/brightness_high.svg"
        }
    }
    RowLayout {
        anchors.fill: parent
        spacing: buttonSpacing

        Repeater{
            model: mask1ToolsModel
            
            GTToolButton{ id: polyBtn
                Layout.fillWidth: true
                Layout.minimumWidth: maskBtnWidth/2
                Layout.maximumWidth: maskBtnWidth/2
                Layout.preferredHeight: itemHeight *2 + 1
                
                text: toolName
                src: toolImg

                Component.onCompleted: {
                    if (toolName == "Polygon") {
                        isActive = Qt.binding(function() { return mask_attrs.polygon_init })
                    }
                }
                
                onClicked: {
                    activeButton = toolName;

                    mask_attrs.mask_shapes_visible = true

                    if (toolName == "Polygon") {
                        mask_attrs.polygon_init = !mask_attrs.polygon_init;
                    } else if (toolName == "Ellipse") {
                        mask_attrs.drawing_action = "Add ellipse";
                    }
                }
            }
        }
        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 2
            Layout.maximumWidth: 2
            Layout.fillHeight: true
        }
        Repeater{
            model: mask2ToolsModel
            
            GTToolButton{ id: polyBtn
                Layout.fillWidth: true
                Layout.minimumWidth: maskBtnWidth/2
                Layout.maximumWidth: maskBtnWidth/2
                Layout.preferredHeight: itemHeight *2 + 1
                
                text: toolName
                src: toolImg
                isActive: activeButton == text
                // Dodge & Burn not implemented yet...
                enabled: false
                
                onClicked: {
                    activeButton = text
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 4
            Layout.fillHeight: true
        }

        GridLayout{ id: propertiesGrid
            rows: 2
            rowSpacing: 1
            columns: 2
            columnSpacing: 1
            flow: GridLayout.TopToBottom

            Layout.fillWidth: true
            Layout.minimumWidth: 40 * columns
            Layout.maximumWidth: maskBtnWidth * columns + columnSpacing
            Layout.maximumHeight: itemHeight * rows + rowSpacing

            XsIntegerAttrControl {
                Layout.preferredWidth: (parent.width - parent.rowSpacing)/propertiesGrid.columns
                Layout.preferredHeight: (parent.height - parent.columnSpacing)/propertiesGrid.rows
                visible: mask_attrs.mask_selected_shape >= 0
                text: "Opacity"
                attr_group_model: mask_attrs.model
                attr_title: "Pen Opacity"
            }
            XsIntegerAttrControl {
                Layout.preferredWidth: (parent.width - parent.rowSpacing)/propertiesGrid.columns
                Layout.preferredHeight: (parent.height - parent.columnSpacing)/propertiesGrid.rows
                visible: mask_attrs.mask_selected_shape >= 0
                text: "Softness"
                attr_group_model: mask_attrs.model
                attr_title: "Pen Softness"
            }
            XsIntegerAttrControl {
                Layout.preferredWidth: (parent.width - parent.rowSpacing)/propertiesGrid.columns
                Layout.preferredHeight: (parent.height - parent.columnSpacing)/propertiesGrid.rows
                visible: activeButton == "Dodge" || activeButton == "Burn"
                text: "Size"
                attr_group_model: mask_attrs.model
                attr_title: "Shapes Pen Size"
            }
            XsPrimaryButton{ id: invertBtn
                Layout.preferredWidth: (parent.width - parent.rowSpacing)/propertiesGrid.columns
                Layout.preferredHeight: (parent.height - parent.columnSpacing)/propertiesGrid.rows
                visible: mask_attrs.mask_selected_shape >= 0
                text: "Invert"
                isActive: mask_attrs.shape_invert
                onClicked:{
                    mask_attrs.shape_invert = !mask_attrs.shape_invert
                }
            }
            XsPrimaryButton{ id: removeBtn
                Layout.preferredWidth: (parent.width - parent.rowSpacing)/propertiesGrid.columns
                Layout.preferredHeight: (parent.height - parent.columnSpacing)/propertiesGrid.rows
                visible: mask_attrs.mask_selected_shape >= 0
                text: "Remove"
                onClicked:{
                    mask_attrs.drawing_action = "Remove shape"
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 4
            Layout.fillHeight: true
        }

        GridLayout{ id: actionGrid
            rows: 2
            rowSpacing: 1
            columns: 2
            columnSpacing: 1

            Layout.fillWidth: true
            Layout.minimumWidth: 40*2
            Layout.maximumWidth: maskBtnWidth + columnSpacing
            Layout.preferredHeight: itemHeight *2 + rowSpacing

            XsPrimaryButton{
                Layout.preferredWidth: (parent.width - actionGrid.rowSpacing)/2
                Layout.preferredHeight: (parent.height - actionGrid.columnSpacing)/2
                text: "Undo"
                imgSrc: "qrc:/icons/undo.svg"
                isActive: false
                enabled: false // Not implemented yet
                
                onClicked: {
                }
            }
            XsPrimaryButton{
                Layout.preferredWidth: (parent.width - actionGrid.rowSpacing)/2
                Layout.preferredHeight: (parent.height - actionGrid.columnSpacing)/2
                text: "Redo"
                imgSrc: "qrc:/icons/redo.svg"
                isActive: false
                enabled: false // Not implemented yet
                
                onClicked: {
                }
            }
            XsPrimaryButton{
                Layout.columnSpan: 2
                Layout.preferredWidth: (parent.width)
                Layout.preferredHeight: (parent.height - actionGrid.columnSpacing)/2
                text: "" //"Reset Layer"
                imgSrc: ""
                imageDiv.width: 18
                imageDiv.height: 18
                enabled: hasActiveGrade()

                RowLayout{
                    width: parent.width
                    height: 16
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 5
                    opacity: parent.enabled || parent.isUnClickable? 1.0 : 0.33

                    Item{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    XsImage {
                        Layout.preferredWidth: parent.height
                        Layout.fillHeight: true
                        antialiasing: true
                        smooth: true
                        imgOverlayColor: palette.text
                        source: "qrc:/icons/rotate-ccw.svg"
                    }
                    XsText {
                        Layout.preferredWidth: textWidth
                        Layout.fillHeight: true
                        text: "Reset All"
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    Item{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    
                }
                
                onClicked: {
                    attrs.grading_action = "Clear"
                }
            }
        }
    }
}