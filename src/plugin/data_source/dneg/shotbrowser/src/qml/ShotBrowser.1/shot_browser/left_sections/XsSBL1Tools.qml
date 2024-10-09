// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudio 1.0
import ShotBrowser 1.0


RowLayout{
    spacing: buttonSpacing
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

    readonly property int butWidth: btnWidth * 0.8

    XsSBLoginDialog{
        id: loginDialog
    }

    XsPrimaryButton{
        id: firstButton
        Layout.minimumWidth: butWidth * 1.5
        Layout.maximumWidth: butWidth * 1.5
        Layout.fillHeight: true
        imgSrc: "qrc:///shotbrowser_icons/nature.svg"
        text: "Tree View"
        isActive: currentCategory == "Tree" && !sequenceTreeShowPresets
        onClicked: {
            if(currentCategory != "Tree")
                resultsBaseModel.setResultData([])

            currentCategory = "Tree"
            sequenceTreeShowPresets = false
        }
    }
    XsPrimaryButton{
        Layout.minimumWidth: firstButton.width
        Layout.maximumWidth: firstButton.width
        Layout.fillHeight: true
        imgSrc: "qrc:///shotbrowser_icons/tree_plus.svg"
        text: "Tree Plus"
        isActive: currentCategory == "Tree" && sequenceTreeShowPresets
        onClicked: {
            if(currentCategory != "Tree")
                resultsBaseModel.setResultData([])
            currentCategory = "Tree"
            sequenceTreeShowPresets = true
        }
    }
    XsPrimaryButton{
        Layout.minimumWidth: firstButton.width
        Layout.maximumWidth: firstButton.width
        Layout.fillHeight: true
        imgSrc: "qrc:///shotbrowser_icons/globe.svg"
        text: "Global View"
        isActive: currentCategory == "Recent"
        onClicked: {
            resultsBaseModel.setResultData([])
            currentCategory = "Recent"
        }
    }
    XsPrimaryButton{
        Layout.minimumWidth: firstButton.width
        Layout.maximumWidth: firstButton.width
        Layout.fillHeight: true
        imgSrc: "qrc:///shotbrowser_icons/settings.svg"
        text: "Setup View"
        isActive: currentCategory == "Menus"
        onClicked: {
            resultsBaseModel.setResultData([])
            currentCategory = "Menus"
        }
    }

    XsComboBoxEditable{
        id: combo
        model: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Project") : []
        textRole: "nameRole"
        Layout.fillWidth: true
        Layout.minimumWidth: btnWidth * 1.3
        Layout.fillHeight: true
        Layout.leftMargin: 4
        Layout.rightMargin: 4
        textField.font.weight: Font.Black

        onActivated: projectIndex = model.index(index, 0)
        onAccepted: {
            projectIndex = model.index(currentIndex, 0)
            focus = false
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

    XsPrimaryButton{ id: credentialsBtn
        Layout.minimumWidth: butWidth
        Layout.maximumWidth: butWidth
        Layout.fillHeight: true
        imgSrc: "qrc:///shotbrowser_icons/manage_accounts.svg"
        isActive: loginDialog.visible
        onClicked: {
            showOrHideLoginDialog()
        }
    }

}
