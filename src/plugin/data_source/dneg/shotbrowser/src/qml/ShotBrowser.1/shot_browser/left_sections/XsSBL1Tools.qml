// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0



Item{ id: toolDiv

    property real categoryBtnWidth: btnWidth * 1.3
    clip: true

    RowLayout{
        spacing: buttonSpacing
        width: parent.width
        height: btnHeight
        anchors.centerIn: parent

        XsPrimaryButton{
            Layout.preferredWidth: btnWidth
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:///shotbrowser_icons/nature.svg"
            text: "Tree"
            isActive: currentCategory == text
            onClicked: {
                currentCategory = text
            }
        }
        XsPrimaryButton{
            Layout.preferredWidth: btnWidth
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:///shotbrowser_icons/globe.svg"
            text: "Presets"
            isActive: currentCategory == "Recent"
            onClicked: {
                currentCategory = "Recent"
            }
        }
        XsPrimaryButton{
            Layout.preferredWidth: btnWidth
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:///shotbrowser_icons/bookmark_heart.svg"
            text: "Menus"
            isActive: currentCategory == text
            onClicked: {
                currentCategory = text
            }
        }
        Item{
            Layout.preferredWidth: buttonSpacing*8
            Layout.preferredHeight: parent.height
        }

        XsComboBoxEditable{
            id: combo
            model: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Project") : []
            textRole: "nameRole"
            Layout.fillWidth: true
            Layout.minimumWidth: categoryBtnWidth
            Layout.preferredHeight: parent.height
            textField.font.weight: Font.Black

            onActivated: projectIndex = model.index(index, 0)
            onAccepted: {
                projectIndex = model.index(currentIndex, 0)
                toolDiv.forceActiveFocus()
            }

            Connections {
                target: panel
                function onProjectIndexChanged() {
                    // console.log(projectIndex, projectIndex.row)
                    if(combo.currentIndex != projectIndex.row) {
                        combo.currentIndex = projectIndex.row
                    }
                }
            }
            Component.onCompleted: {
                if(projectIndex && projectIndex.valid && combo.currentIndex != projectIndex.row) {
                    combo.currentIndex = projectIndex.row
                }
            }
        }

        Item{
            Layout.preferredWidth: buttonSpacing*8
            Layout.preferredHeight: parent.height
        }

        XsPrimaryButton{ id: credentialsBtn
            Layout.preferredWidth: btnWidth
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:///shotbrowser_icons/manage_accounts.svg"
            isActive: loginDialog.visible
            onClicked: {
                showOrHideLoginDialog()
            }
        }

    }

    function showOrHideLoginDialog(){

        if(!loginDialog.visible){
            loginDialog.x = appWindow.x + appWindow.width/3
            loginDialog.y = appWindow.y + appWindow.height/4
            loginDialog.visible = true
        }
        else{
            loginDialog.visible = true
        }

    }

    XsSBLoginDialog{
        id: loginDialog
    }


}