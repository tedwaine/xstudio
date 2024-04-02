// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.14

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.module 1.0
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

XsWindow{

    title: "ShotGrid Authentication"
    property string message: ""

    property real itemHeight: btnHeight
    property real itemSpacing: 5

    width: 460
    height: 240
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    palette.base: XsStyleSheet.panelTitleBarColor

    XsModuleAttributes {
        id: attrs_values
        attributesGroupNames: "shotbrowser_datasource_preference"
    }
    XsModuleAttributes {
        id: attrs_options
        attributesGroupNames: "shotbrowser_datasource_preference"
        roleName: "combo_box_options"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: itemSpacing

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight/2
        }
        XsTextWithComboBox{ id: authMethod
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Authentication Method :"
            property bool ready: false

            onCurrentIndexChanged: {
                if(ready && currentIndex != -1 ) {
                    attrs_values.authentication_method = model[currentIndex]
                }
            }

            model: attrs_options.authentication_method && attrs_options.authentication_method.length ? attrs_options.authentication_method : []
            onModelChanged: {
                if(model.length && attrs_values.authentication_method != undefined) {
                    ready = true
                    currentIndex = valueDiv.find(attrs_values.authentication_method)
                }
            }

            property var auth_method: attrs_values.authentication_method ? attrs_values.authentication_method : null
            onAuth_methodChanged: {
                if(ready && valueDiv.find(attrs_values.authentication_method) != -1 && currentIndex != valueDiv.find(attrs_values.authentication_method)){
                    currentIndex = valueDiv.find(attrs_values.authentication_method)
                }
            }

        }
        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight/2
        }

        XsTextWithInputField{ id: clId
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            label: "Client Identifier :"
            visible: attrs_values.authentication_method == "client_credentials"
            value: attrs_values.client_id ? attrs_values.client_id : null

            onEditingCompleted: {attrs_values.client_id = text}
        }
        XsTextWithInputField{ id: clSecret
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            label: "Client Secret :"
            echoMode: TextInput.Password
            visible: attrs_values.authentication_method == "client_credentials"
            value: attrs_values.client_secret ? attrs_values.client_secret : null

            onEditingCompleted: {attrs_values.client_secret = text}
        }

        XsTextWithInputField{ id: userName
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            label: "Username :"
            visible: attrs_values.authentication_method == "password"
            value: attrs_values.username ? attrs_values.username : null

            onEditingCompleted: {attrs_values.username = text}
        }
        XsTextWithInputField{ id: passWord
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            label: "Password :"
            echoMode: TextInput.Password //PasswordEchoOnEdit
            visible: attrs_values.authentication_method == "password"
            value: attrs_values.password ? attrs_values.password : null

            onEditingCompleted: {attrs_values.password = text}
        }

        XsTextWithInputField{ id: sessToken
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            label: "Session Token :"
            visible: attrs_values.authentication_method == "session_token"
            value: attrs_values.session_token ? attrs_values.session_token : null

            onEditingCompleted: {attrs_values.username = text}
        }
        Item{ id: sessDummy
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            visible: attrs_values.authentication_method == "session_token"
        }

        Item{ id: msgDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            XsText{
                width: parent.width - itemSpacing*2
                height: message? itemHeight : 0
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                color: XsStyleSheet.errorColor
                wrapMode: Text.Wrap
                text: message
            }
        }
        Item{ id: authBtnDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            RowLayout{
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10

                Item{
                    Layout.preferredWidth: parent.width/3
                    Layout.fillHeight: true
                }
                XsPrimaryButton{ id: authBtn
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "Authenticate"
                    onClicked: {
                        ShotBrowserEngine.authenticate(false)

                        forceActiveFocus()
                        if(message == "") close()
                    }
                }
                Item{
                    Layout.preferredWidth: parent.width/3
                    Layout.fillHeight: true
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight/2
        }

    }



}

