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

    width: Math.max(titleRow.width, sliderList.width)

    property string title

    property var colours: ["red", "green", "blue", "white"]

    ColumnLayout {

        spacing: 10

        Row {

            id: titleRow
            spacing: 10
            Layout.alignment: Qt.AlignHCenter
            height: 30

            Text {
                text: root.title
                font.pixelSize: 20
                color: "white"
            }

            XsPrimaryButton {
                id: reloadButton
                width: 20; height: 20
                bgColorNormal: "transparent"
                borderWidth: 0

                onClicked: {
                    value = default_value
                }

                Image {
                    source: "qrc:/feather_icons/rotate-ccw.svg"

                    layer {
                        enabled: true
                        effect: ColorOverlay {
                            color: reloadButton.down || reloadButton.hovered ? palette.brightText : palette.text
                        }
                    }
                }
            }
        }

        Row {
            id: sliderList
            Layout.alignment: Qt.AlignHCenter
            spacing: 10
            Repeater {
                model: value.length
                GradingVSlider {
                    backend_value: value[index]
                    default_backend_value: default_value[index]
                    from: float_scrub_min[index]
                    to: float_scrub_max[index]
                    step: float_scrub_step[index]
                    colour: root.colours[index]
                    onSetValue: {
                        var _value = value;
                        _value[index] = v
                        value = _value
                    }
                }
            }
        }

        Row {

            Layout.alignment: Qt.AlignHCenter

            // stacked text boxes for the R,G,B elements of slope/offset/power
            Column {
                
                id: sliderInputCol
                Repeater {
                    model: value.length-1
                    XsTextField {
                        id: inputRed
                        width: 60
                        bgColorNormal: "transparent"
                        borderColor: bgColorNormal
                        validator: DoubleValidator {
                            bottom: float_scrub_min[index]
                        }
                        text: value[index].toFixed(5)

                        onEditingFinished: {
                            var _value = value;
                            _value[index] = parseFloat(text)
                            value = _value
                        }
                    }
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: 5

                Label {
                    visible: root.title != "Saturation"
                    text: root.title == "Offset" ? "+" : "x"
                }
            }

            // stacked text boxes for the 'master' control of slope/offset/power
            Column {
                anchors.verticalCenter: parent.verticalCenter
                XsTextField {
                    id: inputMaster
                    width: 60
                    bgColorNormal: "transparent"
                    borderColor: bgColorNormal
                    validator: DoubleValidator {
                        bottom: float_scrub_min[float_scrub_min.length-1]
                    }
                    text: value[value.length-1].toFixed(5)

                    onEditingFinished: {
                        var _value = value;
                        _value[value.length-1] = parseFloat(text)
                        value = _value
                }
                }
            }
        }
    }

    Item {
        // spacer
        Layout.fillHeight: true
    }

}
