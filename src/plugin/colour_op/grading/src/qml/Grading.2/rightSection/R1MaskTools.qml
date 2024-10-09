// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14

import xStudio 1.0
import Grading 2.0

import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.bookmarks 1.0

Item{

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
        width: parent.width
        height: btnHeight
        anchors.centerIn: parent
        spacing: buttonSpacing

        Repeater{
            model: mask1ToolsModel

            GTToolButtonHorz{
                Layout.fillWidth: true
                Layout.minimumWidth: maskBtnWidth/1.5
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.preferredHeight: btnHeight
                txt: toolName
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

        // Dodge & Burn not implemented yet...
        // Repeater{
        //     model: mask2ToolsModel
        //     GTToolButton{ id: polyBtn
        //         Layout.fillWidth: true
        //         Layout.minimumWidth: maskBtnWidth/2
        //         Layout.maximumWidth: maskBtnWidth/2
        //         Layout.preferredHeight: itemHeight *2 + 1
        //         text: toolName
        //         src: toolImg
        //         isActive: activeButton == text
        //         enabled: false
        //         onClicked: {
        //             activeButton = text
        //         }
        //     }
        // }

        RowLayout{ id: propertiesGrid
            enabled: mask_attrs.mask_selected_shape >= 0
            spacing: 1

            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight

            XsIntegerAttrControl {
                Layout.minimumWidth: maskBtnWidth/2
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.fillHeight: true
                text: "Opacity"
                attr_group_model: mask_attrs.model
                attr_title: "Pen Opacity"
            }
            XsIntegerAttrControl {
                Layout.minimumWidth: maskBtnWidth/2
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.fillHeight: true
                text: "Softness"
                attr_group_model: mask_attrs.model
                attr_title: "Pen Softness"
            }
            XsIntegerAttrControl {
                Layout.minimumWidth: maskBtnWidth/2
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.fillHeight: true
                visible: activeButton == "Dodge" || activeButton == "Burn"
                text: "Size"
                attr_group_model: mask_attrs.model
                attr_title: "Shapes Pen Size"
            }
            XsPrimaryButton{ id: invertBtn
                Layout.fillWidth: true
                Layout.minimumWidth: maskBtnWidth/2
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.fillHeight: true
                font.pixelSize: XsStyleSheet.fontSize
                text: "Invert"
                isActive: mask_attrs.shape_invert
                onClicked:{
                    mask_attrs.shape_invert = !mask_attrs.shape_invert
                }
            }
            XsPrimaryButton{ id: removeBtn
                Layout.fillWidth: true
                Layout.minimumWidth: maskBtnWidth/2
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.fillHeight: true
                font.pixelSize: XsStyleSheet.fontSize
                text: "Remove"
                onClicked:{
                    mask_attrs.drawing_action = "Remove shape"
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 2
            Layout.fillHeight: true
        }


        RowLayout{ id: headerButtonsGrid
            Layout.fillWidth: true
            Layout.preferredHeight: btnHeight
            spacing: 1

            XsText {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredWidth: maskBtnWidth*0.5
                Layout.maximumWidth: maskBtnWidth*0.5
                Layout.fillHeight: true
                text: "Range:"
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            XsPrimaryButton{
                Layout.fillWidth: true
                Layout.minimumWidth: maskBtnWidth/2
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.fillHeight: true
                font.pixelSize: XsStyleSheet.fontSize

                text: "Single Frame" //isActive ? "Frame (" + attrs.grade_in + ")" : "Frame"
                // tooltip: "Grade applies on the current frame only"
                enabled: hasActiveGrade()
                isActive: attrs.grade_in == attrs.grade_out && attrs.grade_in != -1
                onClicked: {
                    attrs.grade_in = currentPlayhead.mediaFrame
                    attrs.grade_out = currentPlayhead.mediaFrame
                }
            }
            XsPrimaryButton{
                Layout.fillWidth: true
                Layout.minimumWidth: maskBtnWidth/2
                Layout.preferredWidth: maskBtnWidth
                Layout.maximumWidth: maskBtnWidth
                Layout.fillHeight: true
                font.pixelSize: XsStyleSheet.fontSize
                text: "Full Clip"
                // tooltip: "Grade applies on the full duration of the media"
                enabled: hasActiveGrade()
                isActive: {
                    return (
                        (attrs.grade_in == -1 && attrs.grade_out == -1) ||
                        (attrs.grade_in == 0 && attrs.grade_out == currentPlayhead.durationFrames)
                    )
                }
                onClicked: {
                    attrs.grade_in = -1
                    attrs.grade_out = -1
                }
            }


            // XsPrimaryButton{
            //     Layout.preferredWidth: maskBtnWidth/2
            //     Layout.preferredHeight: (parent.height - parent.columnSpacing)/propertiesGrid.rows
            //     text: isActive ? "Set In (" + attrs.grade_in + ")" : "Set In"
            //     imgSrc: "qrc:/grading_icons/input_circle.svg"
            //     // tooltip: "Grade starts at this frame"
            //     enabled: hasActiveGrade()
            //     isActive: {
            //         return (
            //             (attrs.grade_in != -1 && attrs.grade_in != 0) &&
            //             (attrs.grade_in != attrs.grade_out)
            //         )
            //     }
            //     onClicked: {
            //         attrs.grade_in = currentPlayhead.mediaFrame
            //     }
            // }
            // XsPrimaryButton{
            //     Layout.preferredWidth: maskBtnWidth/2
            //     Layout.preferredHeight: (parent.height - parent.columnSpacing)/propertiesGrid.rows
            //     text: isActive ? "Set Out (" + attrs.grade_out + ")" : "Set Out"
            //     imgSrc: "qrc:/grading_icons/output_circle.svg"
            //     // tooltip: "Grade ends at this frame"
            //     enabled: hasActiveGrade()
            //     isActive: {
            //         return (
            //             (attrs.grade_out != -1) &&
            //             (attrs.grade_in != attrs.grade_out)
            //         )
            //     }
            //     onClicked: {
            //         attrs.grade_out = currentPlayhead.mediaFrame
            //     }
            // }

        }

        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 2
            Layout.fillHeight: true
        }

        GTToolButtonHorz{
            Layout.fillWidth: true
            Layout.minimumWidth: maskBtnWidth
            Layout.preferredWidth: maskBtnWidth
            Layout.maximumWidth: maskBtnWidth
            Layout.preferredHeight: btnHeight
            txt: "Reset Layer"
            src: "qrc:/icons/rotate-ccw.svg"
            enabled: hasActiveGrade()

            onClicked: {
                attrs.grading_action = "Clear"
            }
        }
    }
}