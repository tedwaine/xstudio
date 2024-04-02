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
    width: 20
    height: 160

    // Initialize to an arbitrary non-zero value so that we get a changed
    // event when the slider is constructed (eg. Offset slider maps
    // 0.0 to 0.5 position).
    property real backend_value: -1e6
    property real default_backend_value
    property real from
    property real to
    property real step
    property var colour

    onBackend_valueChanged: {
        if (!slider.pressed) {
            slider.value = val_to_pos(backend_value)
        }
    }

    signal setValue(v: real)

    // Note this is a naive log scale, in case the min and max are not
    // mirrored around mid, the derivate will not be continous at the
    // mid point.

    function pos_to_val(v) {
        var min = root.from
        var mid = root.default_backend_value
        var max = root.to
        var steepness = 4

        function lin_to_log(v) {
            var log = Math.log
            var antilog = Math.exp
            return (antilog(v * steepness) - antilog(0.0)) / (antilog(1.0 * steepness) - antilog(0.0))
        }

        if (v < 0.5) {
            return (1 - lin_to_log(1 - v * 2)) * (mid - min) + min
        } else {
            return lin_to_log((v - 0.5) * 2) * (max - mid) + mid
        }
    }

    function val_to_pos(v) {
        var min = root.from
        var mid = root.default_backend_value
        var max = root.to
        var steepness = 4

        function log_to_lin(v) {
            var log = Math.log
            var antilog = Math.exp
            return log(v * (antilog(1.0 * steepness) - antilog(0.0)) + antilog(0.0)) / steepness
        }

        if (v < pos_to_val(0.5, min, mid, max, steepness)) {
            return (1 - log_to_lin(1 - ((v - min) / (mid - min)))) / 2.0
        } else {
            return log_to_lin((v - mid) / (max - mid)) / 2.0 + 0.5
        }
    }

    Slider {
        id: slider
        anchors.fill: parent
        from: 0.0
        to: 1.0
        stepSize: 0.01
        orientation: Qt.Vertical
        focus: true

        onValueChanged: {
            if (pressed) {
                root.setValue(pos_to_val(slider.value))
            }
        }

        background: Rectangle {
            x: slider.leftPadding + slider.availableWidth / 2 - width / 2
            y: slider.topPadding
            width: 4
            height: slider.availableHeight
            color: "grey"
            radius: 2

            Rectangle {
                y: slider.visualPosition * parent.height
                width: parent.width
                height: parent.height - y
                color: root.colour
                radius: 2
            }
        }

        handle: Rectangle {

            id: sliderHandle
            x: (slider.width - width) / 2
            y: slider.topPadding + slider.visualPosition * (slider.availableHeight - height)
            width: 15
            height: 15

            radius: 15
            color: root.colour
            border.color: Qt.darker(root.colour, 4)

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true

                onPressed: {
                    mouse.accepted = mouse.flags & Qt.MouseEventCreatedDoubleClick
                }

                onDoubleClicked: {
                    slider.value = val_to_pos(root.default_backend_value)
                    root.setValue(root.default_backend_value)
                    mouse.accepted = true
                }
            }
        }
    }

}
