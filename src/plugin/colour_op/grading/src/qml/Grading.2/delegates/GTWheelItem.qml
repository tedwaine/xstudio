// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3 //for ColorDialog
import QtGraphicalEffects 1.15 //for RadialGradient

import xStudioReskin 1.0
import xstudio.qml.bookmarks 1.0
import Grading 2.0

Item { 
    clip: true
    property real dividerWidth: 2

    Rectangle{ id: bg
        anchors.fill: parent
        color: XsStyleSheet.baseColor
    }
    Rectangle{ id: divider
        width: dividerWidth
        height: parent.height
        color: XsStyleSheet.panelBgColor
        anchors.left: parent.left
    }

    ColumnLayout { id: col
        anchors.fill: bg
        anchors.margins: 2
        spacing: 1

        Item{ id: titleDiv
            Layout.fillWidth: true
            Layout.preferredHeight: XsStyleSheet.widgetStdHeight 
            Layout.maximumHeight: XsStyleSheet.widgetStdHeight

            // XsGradientRectangle{
            //     width: parent.width+2
            //     height: parent.height
            //     // bottomColor: XsStyleSheet.panelTitleBarColor
            //     bottomColor: "transparent"
            //     bottomPosition: 2
            // }
            XsText {
                text: abbr_title
                font.pixelSize: XsStyleSheet.fontSize*1.2
                font.bold: true
                anchors.centerIn: parent
            }
        }
        
        RowLayout{ id: controlsDiv
            Layout.fillWidth: true 
            Layout.fillHeight: true
            spacing: 1

            Item{
                Layout.fillWidth: true
                Layout.minimumWidth: 4
                Layout.fillHeight: true
            }
    
            ColumnLayout { id: wheelDiv
                Layout.fillWidth: true
                Layout.minimumWidth: defaultWheelSize/2
                Layout.preferredWidth: defaultWheelSize
                Layout.maximumWidth: defaultWheelSize 
                Layout.preferredHeight: parent.height
                spacing: 1

                property int defaultWheelSize: 135
    
                Item{
                    Layout.fillWidth: true
                    Layout.minimumHeight: wheelDiv.defaultWheelSize/2 
                    Layout.preferredHeight: wheelDiv.defaultWheelSize 
                    Layout.fillHeight: true

                    onWidthChanged:{
                        wheel.scale = Math.min(width, height) < wheelDiv.defaultWheelSize ? 
                            Math.min(width, height)/wheelDiv.defaultWheelSize : 1
                    }
                    onHeightChanged:{
                        wheel.scale = Math.min(width, height) < wheelDiv.defaultWheelSize ? 
                            Math.min(width, height)/wheelDiv.defaultWheelSize : 1
                    }
                    
                    GTWheel { 
                        id: wheel
                        x: dividerWidth*6
                        backend_color: value
                        anchors.centerIn: parent
                    }
                }
    
                Item{
                    Layout.fillWidth: true
                    Layout.minimumHeight: 4
                    Layout.fillHeight: true
                }
                
                Repeater {
                    model: 3
    
                    Item{
                        Layout.fillWidth: true
                        Layout.preferredHeight: XsStyleSheet.widgetStdHeight
                        
                        GTValueEditor{
                            width: 40+10
                            height: parent.height
                            anchors.horizontalCenter: parent.horizontalCenter
                            valueText: value[index].toFixed(4)
                            indicatorColor: index==0?"red":index==1?"green":"blue"
    
                            onEdited:{
                                var _value = value
                                _value[index] = parseFloat(currentText)
                                value = _value
                            }
                        }
                    }
                }
    
                Item{
                    Layout.fillWidth: true
                    Layout.minimumHeight: XsStyleSheet.widgetStdHeight + 4
    
                    XsPrimaryButton { 
                        id: resetWheelButton
                        width: 40+10
                        height: XsStyleSheet.widgetStdHeight
                        anchors.horizontalCenter: parent.horizontalCenter
                        
                        imgSrc: "qrc:/icons/rotate-ccw.svg"
                        imageDiv.width: 16
                        imageDiv.height: 16
                        onClicked: {
                            // 'value' and 'default_value' exposed from the model used
                            // to instantiate the wheel
                            var _value = value
                            _value[0] = default_value[0]
                            _value[1] = default_value[1]
                            _value[2] = default_value[2]
                            value = _value
                        }
                    }
                }
                
            }

            Item{
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.fillHeight: true
            }
    
            ColumnLayout { id: sliderDiv
                Layout.minimumWidth: 45 
                Layout.preferredWidth: 45 
                Layout.maximumWidth: 45 
                Layout.preferredHeight: parent.height
                spacing: 1
                
                Item{
                    Layout.fillWidth: true
                    Layout.minimumHeight: 2
                }

                Item{
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    GTSlider{ 
                        id: slider
                        height: parent.height
                        anchors.horizontalCenter: parent.horizontalCenter
                        backend_value: value[3]
                        default_backend_value: default_value[3]
                        from: float_scrub_min[3]
                        to: float_scrub_max[3]
                        step: float_scrub_step[3]
        
                        onSetValue: {
                            var _value = value
                            _value[3] = newVal
                            value = _value
                        }
                    }
                }
    
                Item{
                    Layout.fillWidth: true
                    Layout.minimumHeight: XsStyleSheet.widgetStdHeight
                    
                    GTValueEditor{
                        width: parent.width 
                        height: parent.height
                        valueText: value[3].toFixed(3)
                        indicatorColor: "transparent"
    
                        onEdited:{
                            var _value = value
                            _value[3] = parseFloat(currentText)
                            value = _value
                        }
                    }
                }

                Item{
                    Layout.fillWidth: true
                    Layout.minimumHeight: XsStyleSheet.widgetStdHeight + 4
                    
                    XsPrimaryButton { 
                        id: resetSliderButton
                        width: parent.width
                        height: XsStyleSheet.widgetStdHeight
        
                        imgSrc: "qrc:/icons/rotate-ccw.svg"
                        imageDiv.width: 16
                        imageDiv.height: 16
                        onClicked: {
                            var _value = value
                            _value[3] = default_value[3]
                            value = _value
                        }
                    }
                }
    
            }

            Item{
                Layout.fillWidth: true
                Layout.minimumWidth: 4
                Layout.fillHeight: true
            }

        }
        
    }
    
}