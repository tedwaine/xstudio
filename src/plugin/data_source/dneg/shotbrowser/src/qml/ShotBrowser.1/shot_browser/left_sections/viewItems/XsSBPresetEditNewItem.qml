// SPDX-License-Identifier: Apache-2.0

import QtQuick

import QtQuick.Layouts

import xStudio 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

Rectangle {
    id: control
    property var termModel: null
    property var delegateModel: null
    color: XsStyleSheet.widgetBgNormalColor

    RowLayout {
        anchors.fill: parent
        spacing: 1

        Item{
            Layout.maximumWidth: dragWidth
            Layout.minimumWidth: dragWidth
            Layout.fillHeight: true
        }
        Item {
            Layout.maximumWidth: enableWidth
            Layout.minimumWidth: enableWidth
            Layout.fillHeight: true
        }

        XsComboBox {
            Layout.maximumWidth: termWidth
            Layout.minimumWidth: termWidth
            Layout.preferredHeight: control.height

            model: termModel
            displayText: currentIndex == -1? "Select Term..." : currentText

            currentIndex: -1
            onModelChanged: {
                // console.log("new item", "onModelChanged", model)
                currentIndex = -1
            }

            // don't use onCurrentIndex changed! As that'll get programatic changes as well.
            onActivated: {
                if(index != -1) {
                    let parent_index = control.delegateModel.newTermParent
                    let row = ShotBrowserEngine.presetsModel.rowCount(parent_index)

                    // console.log(parent_index, row, presetDelegateModel.newTermParent)

                    let i = ShotBrowserEngine.presetsModel.insertTerm(
                        textAt(index),
                        row,
                        parent_index
                    )

                    control.delegateModel.model.expand(parent_index)

                    if(i.valid) {
                        let t = ShotBrowserEngine.presetsModel.get(i, "termRole")
                        let tm = ShotBrowserEngine.presetsModel.termModel(t, entityType, projectId)
                        if(tm.length && tm.get(tm.index(0,0), "nameRole") == "True") {
                            ShotBrowserEngine.presetsModel.set(i, "True", "valueRole")
                        }
                    }
                    currentIndex = -1
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}

