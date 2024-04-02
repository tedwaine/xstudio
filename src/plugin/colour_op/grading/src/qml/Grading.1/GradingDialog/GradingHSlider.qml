// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3 //for ColorDialog
import QtGraphicalEffects 1.15 //for RadialGradient

import xStudioReskin 1.0

Item {
    id: root

    width: sliderRow.width
    height: sliderRow.height

    property string title
    
    property var backend_value
    property var default_value
    property var from
    property var to
    property var step
    property var colour: "white"

    property bool linear_scale: false

    onBackend_valueChanged: {
        if (!slider.pressed) {
            slider.value = val_to_pos(backend_value)
        }
    }

    signal setValue(newValue: real)

    // Note this is a naive log scale, in case the min and max are not
    // mirrored around mid, the derivate will not be continous at the
    // mid point.

    function pos_to_val(v) {
        var min = root.from
        var mid = root.default_value
        var max = root.to
        var steepness = 4

        function lin_to_log(v) {
            var log = Math.log
            var antilog = Math.exp
            return (antilog(v * steepness) - antilog(0.0)) / (antilog(1.0 * steepness) - antilog(0.0))
        }

        if (root.linear_scale) {
            if (v < 0.5) {
                return (1 - (1 - v * 2)) * (mid - min) + min
            } else {
                return ((v - 0.5) * 2) * (max - mid) + mid
            }
        } else {
            if (v < 0.5) {
                return (1 - lin_to_log(1 - v * 2)) * (mid - min) + min
            } else {
                return lin_to_log((v - 0.5) * 2) * (max - mid) + mid
            }
        }
    }

    function val_to_pos(v) {
        var min = root.from
        var mid = root.default_value
        var max = root.to
        var steepness = 4

        function log_to_lin(v) {
            var log = Math.log
            var antilog = Math.exp
            return log(v * (antilog(1.0 * steepness) - antilog(0.0)) + antilog(0.0)) / steepness
        }

        if (linear_scale) {
            if (v < pos_to_val(0.5, min, mid, max, steepness)) {
                return (1 - (1 - ((v - min) / (mid - min)))) / 2.0
            } else {
                return ((v - mid) / (max - mid)) / 2.0 + 0.5
            }
        } else {
            if (v < pos_to_val(0.5, min, mid, max, steepness)) {
                return (1 - log_to_lin(1 - ((v - min) / (mid - min)))) / 2.0
            } else {
                return log_to_lin((v - mid) / (max - mid)) / 2.0 + 0.5
            }
        }
    }

    Row {
        id: sliderRow
        spacing: 15

        Text {
            text: root.title
            font.pixelSize: 15
            color: "white"
            width: 80
        }

        XsPrimaryButton {
            id: reloadButton
            width: 15; height: 15
            bgColorNormal: "transparent"
            borderWidth: 0

            onClicked: {
                setValue(default_value)
            }

            Image {
                source: "qrc:/feather_icons/rotate-ccw.svg"
                sourceSize.width: 15
                sourceSize.height: 15

                layer {
                    enabled: true
                    effect: ColorOverlay {
                        color: reloadButton.down || reloadButton.hovered ? palette.brightText : palette.text
                    }
                }
            }
        }

        Slider {
            id: slider
            width: 400
            from: 0.0
            to: 1.0
            // Adjust to make sure the desired step size is achieved after
            // linear interpolation in the pos_to_val and val_to_pos functions
            stepSize: root.linear_scale ? (root.step - root.default_value) / (2 * (root.to - root.default_value)) : 0.01
            orientation: Qt.Horizontal

            onValueChanged: {
                if (pressed) {
                    setValue(pos_to_val(slider.value))
                }
            }

            background: Rectangle {
                x: slider.leftPadding
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                width: slider.availableWidth
                height: 4
                radius: 2
                color: "grey"

                Rectangle {
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    color: "white"
                    radius: 2
                }
            }

            handle: Rectangle {
                id: sliderHandle

                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: (slider.height - height) / 2
                width: 15
                height: 15

                radius: 15
                color: root.colour
                border.color: "white"

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    propagateComposedEvents: true

                    onPressed: {
                        mouse.accepted = mouse.flags & Qt.MouseEventCreatedDoubleClick
                    }

                    onDoubleClicked: {
                        slider.value = val_to_pos(root.default_value)
                        root.setValue(root.default_value)
                        mouse.accepted = true
                    }
                }
            }
        }

        XsTextField {
            id: sliderInput
            width: 60
            bgColorNormal: "transparent"
            borderColor: bgColorNormal
            text: root.backend_value.toFixed(5)

            validator: DoubleValidator {
                bottom: from
                top: to
            }

            onFocusChanged: {
                if(focus) {
                    selectAll()
                    forceActiveFocus()
                }
                else {
                    deselect()
                }
            }

            onEditingFinished: {
                setValue(parseFloat(text))
            }
        }
    }
}
