// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

Item{
    id: control
    property var termModel: null
    property var delegateModel: null

    Rectangle{ id: bgDiv
        anchors.fill: parent;
        color: XsStyleSheet.widgetBgNormalColor
        opacity: 0.5
    }

    RowLayout {
        width: parent.width
        height: parent.height -1
        spacing: 1

        Item{
            Layout.preferredWidth: btnWidth/3
            Layout.fillHeight: true
        }
        Item {
            Layout.preferredWidth: height
            Layout.fillHeight: true
        }

        XsComboBox {
            Layout.preferredWidth: btnWidth*4.2
            Layout.fillHeight: true

            model: termModel
            displayText: currentIndex == -1? "Select Term..." : currentText

            currentIndex: -1
            onModelChanged: {
                console.log("new item", "onModelChanged", model)
                currentIndex = -1
            }

            // don't use onCurrentIndex changed! As that'll get programatic changes as well.
            onActivated: {
                if(index != -1) {
                    let parent_index = control.delegateModel.newTermParent
                    let row = ShotBrowserEngine.presetsModel.rowCount(parent_index)

                    console.log(parent_index, row, presetDelegateModel.newTermParent)

                    let i = ShotBrowserEngine.presetsModel.insertTerm(
                        textAt(index),
                        row,
                        parent_index
                    )

                    control.delegateModel.model.expand(parent_index)

                    if(i.valid) {
                        let t = ShotBrowserEngine.presetsModel.get(i, "termRole")
                        let tm = ShotBrowserEngine.presetsModel.termModel(t, entityType, projectPref.value)
                        if(tm.length && tm.get(tm.index(0,0), "nameRole") == "True") {
                            ShotBrowserEngine.presetsModel.set(i, "True", "valueRole")
                        }
                    }
                    currentIndex = -1
                }
            }
        }
        Item{
            Layout.preferredWidth: height
            Layout.fillHeight: true
        }
        Item{
            Layout.fillWidth: true
            Layout.preferredWidth: btnWidth*4.2
            Layout.fillHeight: true
        }
        Item{
            Layout.preferredWidth: btnWidth
            Layout.fillHeight: true
        }

    }
}

