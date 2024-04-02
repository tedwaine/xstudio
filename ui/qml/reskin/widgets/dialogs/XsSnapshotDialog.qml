// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.0

import xstudio.qml.module 1.0
import xstudio.qml.models 1.0
import xStudioReskin 1.0

Window {

	id: dialog
	width: 450
	height: 300

    property int imageSourceWidth: 1920
    property int imageSourceHeight: 1080

    maximumHeight: height
    maximumWidth: width

    minimumHeight: height
    minimumWidth: width
    color: "transparent"

    title: "Viewer Snapshot"

    XsGradientRectangle{ id: bgDiv
        z: -10
        anchors.fill: parent
    }

    // parent: sessionWidget
    // x: Math.max(0, (sessionWidget.width - width) / 2)
    // y: (sessionWidget.height - height) / 2
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        Rectangle {
            color: "transparent"
            Layout.fillHeight: true
        }

        FileDialog {
            id: filedialog
            title: qsTr("Name / select a file to save")
            selectMultiple: false
            selectFolder: false
            selectExisting: false
            nameFilters: [ "JPEG files (*.jpg)", "PNG files (*.png)", "TIF files (*.tif *.tiff)", "EXR files (*.exr)" ]
            property var suffixes: ["jpg", "png", "tif", "exr"]
            defaultSuffix: "jpg"
            selectedNameFilter: "JPEG files (*.jpg)"
            onAccepted: {
                var fixedfileUrl = fileUrl.toString()
                var result = studio.renderScreenShotToDisk(
                    fixedfileUrl,
                    0,
                    parseInt(widthInput.text),
                    parseInt(heightInput.text))
                if (result != "") {
                    dialogHelpers.errorDialogFunc("Snapshot Failed", result)
                } else {
                    dlg.close()
                }
            }
        }

        Rectangle {
            color: "transparent"
            height: 160
            Layout.fillWidth: true

            GridLayout {
                id: main_layout
                columns: 2
                rows: 2
                rowSpacing: 12//20
                anchors.centerIn: parent

                XsLabel {
                    text: "Image Size :"
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                }

                XsComboBox {
                    id: resolutionChoice
                    model:
                    ListModel {
                        id: imgModel
                        ListElement {
                            text: "Original"
                        }
                        ListElement {
                            text: "Half"
                        }
                        ListElement {
                            text: "xStudio Viewport Size"
                        }
                        ListElement {
                            text: "User defined"
                        }
                    }
                    width: 200
                    height: 24
                    onActivated: {
                        widthInput.enabled = currentIndex == 3
                        heightInput.enabled = currentIndex == 3
                        widthInput.text = currentIndex == 0 ? imageSourceWidth: currentIndex == 1 ? Math.round(imageSourceWidth / 2) : currentIndex == 2 ? viewerWidth : widthInput.text
                        heightInput.text = currentIndex == 0 ? imageSourceHeight: currentIndex == 1 ? Math.round(imageSourceHeight / 2) : currentIndex == 2 ? viewerHeight : viewerHeight.text
                    }
                }

                Rectangle{color: "transparent"; width:10; height:10}
                RowLayout {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    XsLabel {
                        text: "Width :"
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        opacity: widthInput.enabled ? 1: 0.5
                    }
                    XsTextField {
                        // anchors.fill: parent
                        id: widthInput
                        text: imageSourceWidth
                        // width: font.pixelSize*2
                        width: 50
                        height: 24
                        enabled: false
                        color: enabled ? XsStyleSheet.controlColor: XsStyleSheet.controlColorDisabled
                        selectByMouse: true
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        validator: IntValidator{bottom: 1}

                        font {
                            family: XsStyleSheet.fontFamily
                        }

                        onEditingFinished: {
                            focus = false
                        }
                    }

                    XsLabel {
                        text: "Height :"
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        opacity: heightInput.enabled ? 1: 0.5
                    }
                    XsTextField {
                        // anchors.fill: parent
                        id: heightInput
                        text: imageSourceHeight
                        // width: font.pixelSize*2
                        width: 50
                        height: 24
                        enabled: false
                        color: enabled ? XsStyleSheet.controlColor: XsStyleSheet.controlColorDisabled
                        selectByMouse: true
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        validator: IntValidator{bottom: 1}

                        font {
                            family: XsStyleSheet.fontFamily
                        }

                        onEditingFinished: {
                            focus = false
                        }
                    }
                }

                XsLabel {
                    text: "OCIO Display:"
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                }

                XsAttrComboBox {
                    Layout.fillWidth: true
                    Layout.preferredHeight: XsStyleSheet.widgetStdHeight
                    attr_title: "Display"
                    attr_model_name: "offscreen_viewport1_toolbar"
                }
            
                XsLabel {
                    text: "OCIO View:"
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                }

                XsAttrComboBox {
                    Layout.fillWidth: true
                    Layout.preferredHeight: XsStyleSheet.widgetStdHeight
                    attr_title: "View"
                    attr_model_name: "offscreen_viewport1_toolbar"
                }

            }
        }

        Rectangle {
            color: "transparent"
            Layout.fillHeight: true
        }

        RowLayout {

            Layout.alignment: Qt.AlignRight
            Layout.fillHeight: false
            spacing: 10

            XsPrimaryButton {
                id: btnCancel
                text: qsTr("Cancel")
                implicitHeight: 24
                onClicked: accept()
            }

            XsPrimaryButton {
                id: btnSave
                text: qsTr("Save Snapshot ...")
                implicitHeight: 24
                onClicked: filedialog.open()
            }

            XsPrimaryButton {
                id: btnToClipboard
                text: qsTr("Snapshot to Clipboard")
                implicitHeight: 24
                onClicked: {
                    studio.renderScreenShotToClipboard(
                        parseInt(widthInput.text),
                        parseInt(heightInput.text))
                }
            }

        }
    }
}