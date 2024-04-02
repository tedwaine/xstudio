// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Dialogs 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import QuickFuture 1.0
import QuickPromise 1.0

import xStudioReskin 1.0

Item {

    id: root

    Loader {
        id: loader
    }

    function hideLastDialog() {
        loader.item.visible = false
    }
    
    function showDialog(callback) {
        loader.item.x = root.width/2 - loader.item.width/2
        loader.item.y = root.height/2 - loader.item.height/2
        if (callback) loader.item.response.connect(callback)
        loader.item.visible = true
    }

    property var dialogVisible: loader.item ? loader.item.visible != undefined ? loader.item.visible : false : false

    onDialogVisibleChanged: {
        if (!visible) appWindow.grabFocus()
    }

    /**************************************************************

    FileDialog

    ****************************************************************/

    Component {
        id: fileDialog

        FileDialog {
            property var chaser
            
            onAccepted: {
                if (selectMultiple)
                    result(fileUrls, folder, chaser)
                else
                    result(fileUrl, folder, chaser)
                // unload (important!)
                loader.sourceComponent = undefined
            }
            onRejected: {
                result(false)
                // unload (important!)
                loader.sourceComponent = undefined
            }
            signal result(variant _path, variant _folder, variant _chaser);
        }
    }

    function showFileDialog(
        resultCallback,
        folder,
        title,
        defaultSuffix,
        nameFilters,
        selectExisting,
        selectMultiple,
        chaser) {

        loader.sourceComponent = fileDialog
        loader.item.result.connect(resultCallback)
        if (folder) loader.item.folder = folder
        loader.item.title = title
        loader.item.defaultSuffix = defaultSuffix ? defaultSuffix : ""
        loader.item.nameFilters = nameFilters
        loader.item.selectExisting = selectExisting
        loader.item.selectMultiple = selectMultiple
        loader.item.chaser = chaser
        loader.item.visible = true
    }

    /**************************************************************

    xSTUDIO Simple Error Message Box

    ****************************************************************/

    Component {
        id: errorDialog
        XsPopup {

            property string title: "Error"
            property string body: ""
            width: 400
            height: 200
            id: errorDialog

            ColumnLayout {

                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                Text {
                    Layout.fillWidth: true
                    text: title
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 20
                    Image {
                        id: thumbnailImgDiv
                        width: 40
                        height: 40
                        source: "qrc:/icons/error.svg"                
                    }
                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: body
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }
                }
                XsSimpleButton {
                    text: "Close"
                    Layout.alignment: Qt.AlignRight
                    onClicked: {
                        errorDialog.visible = false
                    }
                }
            }
            onVisibleChanged: {
                if (!visible) {
                    loader.sourceComponent = undefined
                }
            }
            background: XsGradientRectangle{
            }
        }
    }

    function errorDialogFunc(error_title, error_body) {
        loader.sourceComponent = errorDialog
        loader.item.title = error_title
        loader.item.body = error_body
        showDialog(undefined)
    }

    /**************************************************************

    xSTUDIO Simple Mutli-Choice Dialog Box

    ****************************************************************/

    Component {

        id: multiChoice
        XsPopup {

            property string title: ""
            property string body: ""
            property var choices: []
            width: 400
            height: 200
            id: popup
            signal response(variant p, variant chaser)
            property var chaser

            ColumnLayout {

                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                Text {
                    Layout.fillWidth: true
                    text: title
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: body
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    Repeater {
                        model: popup.choices
                        XsSimpleButton {
                            text: popup.choices[index]
                            Layout.alignment: Qt.AlignRight
                            onClicked: {
                                popup.response(popup.choices[index], popup.chaser)
                                popup.visible = false
                            }
                        }
                    }
                }
            }
            onVisibleChanged: {
                if (!visible) {
                    loader.sourceComponent = undefined
                }
            }
            background: XsGradientRectangle{
            }
        }
    }

    function multiChoiceDialog(
        callback,
        title,
        body,
        choices,
        chaserFunc)
    {
        loader.sourceComponent = multiChoice
        loader.item.title = title
        loader.item.body = body
        loader.item.choices = choices
        loader.item.chaser = chaserFunc
        showDialog(callback)
    }

    /**************************************************************

    xSTUDIO Simple Text Entry Dialog Box

    ****************************************************************/

    Component {

        id: textInput
        XsPopup {

            property string title: ""
            property string body: ""
            property string initialText: ""
            property var choices: []
            width: 400
            height: 200
            id: popup
            signal response(variant text, variant button_press)
            property var chaser

            Rectangle {
                id: titleBar
                anchors.top: parent.top                
                anchors.left: parent.left
                anchors.right: parent.right
                height: 30
                color: XsStyleSheet.menuBarColor
                XsText {
                    anchors.centerIn: parent
                    text: title
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    font.bold: true
                    font.pixelSize: XsStyleSheet.fontSize*1.2
                }    
            }
            ColumnLayout {

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.top: titleBar.bottom
                anchors.margins: 20
                spacing: 20

                XsText {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: body
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "transparent"
                    border.color: "black"
                    XsTextInput {
                        anchors.fill: parent
                        id: input
                        text: initialText
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    Repeater {
                        model: popup.choices
                        XsSimpleButton {
                            text: popup.choices[index]
                            Layout.alignment: Qt.AlignRight
                            onClicked: {
                                popup.response(input.text, popup.choices[index])
                                popup.visible = false
                            }
                        }
                    }
                }
            }
            onVisibleChanged: {
                if (!visible) {
                    loader.sourceComponent = undefined
                }
            }
            background: XsGradientRectangle{
            }
        }
    }

    function textInputDialog(
        callback,
        title,
        body,
        initialText,
        choices)
    {
        loader.sourceComponent = textInput
        loader.item.title = title
        loader.item.body = body
        loader.item.initialText = initialText
        loader.item.choices = choices
        showDialog(callback)
    }

}