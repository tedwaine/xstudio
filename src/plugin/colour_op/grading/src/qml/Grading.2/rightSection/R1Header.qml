// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

Rectangle{
    color: "transparent"

    property real headerBtnWidth: 100

    property real itemHeight: XsStyleSheet.widgetStdHeight

    RowLayout {
        anchors.fill: parent
        spacing: buttonSpacing

        XsPrimaryButton{
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: headerBtnWidth
            Layout.preferredHeight: itemHeight
            text: "Bypass All"
            imgSrc: ""
            //tooltip: "Bypass all CDLs or not"
            isActive: attrs.grading_bypass
            
            onClicked: {
                attrs.grading_bypass = !attrs.grading_bypass
            }
        }
        XsPrimaryButton{
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: headerBtnWidth
            Layout.preferredHeight: itemHeight
            text: "Hide Shapes"
            isActive: !mask_attrs.mask_shapes_visible
            onClicked:{
                mask_attrs.mask_shapes_visible = !mask_attrs.mask_shapes_visible;
            }
        }
        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 4
            Layout.preferredHeight: itemHeight
        }
        XsPrimaryButton{
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: headerBtnWidth
            Layout.preferredHeight: itemHeight
            text: isActive ? "Frame (" + attrs.grade_in + ")" : "Frame"
            imgSrc: ""
            // tooltip: "Grade applies on the current frame only"
            enabled: hasActiveGrade()
            isActive: attrs.grade_in == attrs.grade_out && attrs.grade_in != -1
            onClicked: {
                attrs.grade_in = currentPlayhead.mediaFrame
                attrs.grade_out = currentPlayhead.mediaFrame
            }
        }
        XsPrimaryButton{ id: fullClipBtn
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: headerBtnWidth
            Layout.preferredHeight: itemHeight
            text: "Full Clip"
            imgSrc: ""
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
        Item{
            Layout.preferredWidth: 4
            Layout.preferredHeight: itemHeight
        }
        XsPrimaryButton{
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: headerBtnWidth
            Layout.preferredHeight: itemHeight
            text: isActive ? "Set In (" + attrs.grade_in + ")" : "Set In"
            imgSrc: ""
            // tooltip: "Grade starts at this frame"
            enabled: hasActiveGrade()
            isActive: {
                return (
                    (attrs.grade_in != -1 && attrs.grade_in != 0) &&
                    (attrs.grade_in != attrs.grade_out)
                )
            }
            onClicked: {
                attrs.grade_in = currentPlayhead.mediaFrame
            }
        }
        XsPrimaryButton{
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: headerBtnWidth
            Layout.preferredHeight: itemHeight
            text: isActive ? "Set Out (" + attrs.grade_out + ")" : "Set Out"
            imgSrc: ""
            // tooltip: "Grade ends at this frame"
            enabled: hasActiveGrade()
            isActive: {
                return (
                    (attrs.grade_out != -1) &&
                    (attrs.grade_in != attrs.grade_out)
                )
            }
            onClicked: {
                attrs.grade_out = currentPlayhead.mediaFrame
            }
        }
        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 4
            Layout.preferredHeight: itemHeight
        }
        XsComboBox { 
            Layout.fillWidth: true
            Layout.minimumWidth: 40*2
            Layout.maximumWidth: headerBtnWidth*1.5
            Layout.preferredHeight: itemHeight
            enabled: hasActiveGrade() && attrs.media_colour_managed
            model: ["raw", "scene_linear", "compositing_log"]
            property var currentColorSpace: attrs.media_colour_managed ? attrs.colour_space : "raw"
            currentIndex: indexOfValue(currentColorSpace)
            onCurrentValueChanged: {
                if (currentValue != "raw")
                    attrs.colour_space = currentValue
            }
        }
    }
}