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
import xstudio.qml.models 1.0

Item {

    id: root
    width: 500
    height: 30    

    Column {
        anchors.fill: parent
        anchors.topMargin: 50
        anchors.leftMargin: 15
        spacing: 10

        GradingHSlider {

            title: "Exposure"
            backend_value: Math.log2(attrs.slope.value[3])
            default_value: Math.log2(attrs.slope.default_value[3])
            from: -6.0
            to: 6.0
            step: 0.01
            linear_scale: true

            onSetValue: {
                var _value = attrs.slope.value
                _value[3] = Math.pow(2,newValue)
                attrs.slope.value = _value
            }
        }

        GradingHSlider {

            title: "Offset"
            backend_value: attrs.offset.value[3]
            default_value: attrs.offset.default_value[3]
            from: attrs.offset.float_scrub_min[3]
            to: attrs.offset.float_scrub_max[3]
            step: 0.001

            onSetValue: {
                var _value = attrs.offset.value
                _value[3] = newValue
                attrs.offset.value = _value
            }
        }

        GradingHSlider {

            title: "Power"
            backend_value: attrs.power.value[3]
            default_value: attrs.power.default_value[3]
            from: attrs.power.float_scrub_min[3]
            to: attrs.power.float_scrub_max[3]
            step: 0.001

            onSetValue: {
                var _value = attrs.power.value
                _value[3] = newValue
                attrs.power.value = _value
            }
        }

        GradingHSlider {

            title: "Sat."
            backend_value: attrs.saturation.value
            default_value: attrs.saturation.default_value
            from: attrs.saturation.float_scrub_min
            to: attrs.saturation.float_scrub_max
            step: 0.001

            onSetValue: {
                attrs.saturation.value = newValue
            }
        }


    }
}
