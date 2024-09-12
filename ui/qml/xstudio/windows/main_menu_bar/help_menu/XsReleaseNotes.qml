// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import QtQuick.Layouts 1.3
import xstudio.qml.helpers 1.0

import xStudio 1.0

XsWindow {
    id: dlg

    height: 600
    width: 800

    XsModelProperty {
        id: features
        role: "valueRole"
        index: app_window.globalStoreModel.searchRecursive("/ui/qml/features", "pathRole")
    }

    property string myHTMLTitle: "<h1>" + Qt.application.name + " v" + Qt.application.version + " Change Log</h1>"
    property string feature_text: ""

    Component.onCompleted: {
        if(features.value.length) {
           var xhr = new XMLHttpRequest;
            xhr.open('GET', features.value+Qt.application.version);
                xhr.onreadystatechange = function() {
                    if (xhr.readyState === XMLHttpRequest.DONE) {
                        var response = xhr.responseText;
                        const regex3 = /<a[^>]+>([^<]+)<\/a>/ig;
                        response = response.replace(regex3, '$1');
                        feature_text = response
                    }
                };
            xhr.send();
            // feature_text = features.value + Qt.application.version
        } else {
            let t = helpers.readFile(studio.releaseDocsUrl())
            const regex1 = /^[^@]+<div class="section" id="release-notes">/im;
            t = t.replace(regex1, '');

            const regex2 = /<footer>[^@]+$/im;
            t = t.replace(regex2, '');

            const regex3 = /<a[^>]+>[^<]+<\/a>/ig;
            t = t.replace(regex3, '');
            feature_text = t
        }



    //    var xhr = new XMLHttpRequest;
    //     xhr.open('GET', "http://dnet.dneg.com/x/deTJJQ");
    //         xhr.onreadystatechange = function() {
    //             if (xhr.readyState === XMLHttpRequest.DONE) {
    //                 var response = xhr.responseText;
    //                 helpText.text = response
    //             }
    //         };
    //     xhr.send();
    }


    Item {
        id: wrapper

        anchors.fill: parent
        // anchors.topMargin: 10
        // anchors.rightMargin: 10
        // anchors.leftMargin: 10
        // anchors.bottomMargin: 10

        Image {
            id: xIcon
            source: "qrc:/images/xstudio_logo_256_v1.svg"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.topMargin: 10
            sourceSize.height: 128
            sourceSize.width: 128
            height: 128
            width: 128
        }

        TextEdit{
            id: titleText
            wrapMode: TextEdit.Wrap
            anchors.bottom: xIcon.bottom
            anchors.left: xIcon.right
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.bottomMargin: 0
            readOnly:true

            text: myHTMLTitle
            textFormat:TextArea.RichText
            font.pixelSize: 16
            font.family: XsStyleSheet.fontFamily
            font.hintingPreference: Font.PreferNoHinting
            color:"#fff"
        }

        Rectangle {
            height: 1
//                width: 100
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.bottom: flickArea.top
            anchors.bottomMargin: 10
            // gradient: styleGradient.accent_gradient
            color: XsStyleSheet.accentColor
        }

        Flickable {
            id: flickArea
            anchors.top: xIcon.bottom
            anchors.topMargin: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: foot.top
            anchors.bottomMargin: 20
            anchors.leftMargin: 10
            anchors.rightMargin: 7
            height: 300
            implicitHeight: 300
            contentWidth: helpText.width
            contentHeight: helpText.height

            flickableDirection: Flickable.VerticalFlick
            clip: true

            TextEdit{
                id: helpText
                wrapMode: TextEdit.Wrap
                anchors.top: parent.top
                anchors.left: parent.left
                width: 700
                readOnly:true
                text: feature_text
                textFormat: TextArea.RichText
                font.pixelSize: 12
                font.family: XsStyleSheet.fontFamily
                font.hintingPreference: Font.PreferNoHinting
                color:"#fff"
            }
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }
        }

        Rectangle {
            height: 1
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.top: flickArea.bottom
            anchors.topMargin: 10
            // gradient: styleGradient.accent_gradient
            color: XsStyleSheet.accentColor
        }

        Rectangle {

            id:foot
            height: 40
            color: "transparent"
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.topMargin: 20

            XsPrimaryButton{
                id: btnOK
                text: qsTr("Okay")
                width: XsStyleSheet.primaryButtonStdWidth*2
                height: XsStyleSheet.primaryButtonStdHeight

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 8

                onClicked: {
                    close()
                }
            }
        }
    }
}