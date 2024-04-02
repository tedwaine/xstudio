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

    width: titleRow.width
    property string title

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

        GradingVSlider 
        {
            id: slider
            Layout.alignment: Qt.AlignHCenter
            backend_value: value
            default_backend_value: default_value
            from: float_scrub_min
            to: float_scrub_max
            step: float_scrub_step
            colour: "white"
            onSetValue: {
                value = v
            }
        }

        XsTextField {

            id: inputMaster
            Layout.alignment: Qt.AlignHCenter
            width: 60
            bgColorNormal: "transparent"
            borderColor: bgColorNormal
            validator: DoubleValidator {
                bottom: float_scrub_min
            }
            text: value.toFixed(5)

            onEditingFinished: {
                value = parseFloat(text)
            }
        }

    }
}