// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import QtQml.Models 2.14
import QtGraphicalEffects 1.15

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import Grading 2.0

Item{
    clip: true

    property string titleText: ""
    
    ColumnLayout{
        anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 2
        spacing: 1

        Item{ id: titleSlider
            Layout.fillWidth: true
            Layout.preferredHeight: XsStyleSheet.widgetStdHeight
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight
            
            // XsGradientRectangle{
            //     width: parent.width+2
            //     height: parent.height
            //     // bottomColor: XsStyleSheet.panelTitleBarColor
            //     bottomColor: "transparent"
            //     bottomPosition: 2
            // }
            XsText {
                text: titleText
                width: parent.width - spacing*2
                elide: Text.ElideRight
                font.pixelSize: XsStyleSheet.fontSize*1.2
                font.bold: true
                anchors.centerIn: parent
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.minimumHeight: 2
        }

        Item{ id: controlDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Layout.preferredHeight: 140 + XsStyleSheet.widgetStdHeight*2
            
            GTSlider{ id: slider
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter

                backend_value: value
                onSetValue: {
                    var _value = (typeof value == "number") ? value : value[index]
                    _value = newVal
                    if(typeof value == "number") value = _value
                    else value[index] = _value
                }
            }
        }

        Item{ id: valuesDiv
            Layout.fillWidth: true
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight

            GTValueEditor{
                width: 45 //parent.width
                height: XsStyleSheet.widgetStdHeight
                valueText: value.toFixed(3)
                indicatorColor: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter

                onEdited:{
                    value = parseFloat(currentText)
                }
            }
        }

        Item{ 
            Layout.fillWidth: true
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight + 4

            XsPrimaryButton { id: resetBtn
                width: 45 
                height: XsStyleSheet.widgetStdHeight
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                
                imgSrc: "qrc:/icons/rotate-ccw.svg"
                imageDiv.width: 16
                imageDiv.height: 16
                onClicked: { 

                    value = default_value

                    // var _value = value
                    // _value[index] = default_value[index]
                    // value = _value
                }
            }
        }

    }

}