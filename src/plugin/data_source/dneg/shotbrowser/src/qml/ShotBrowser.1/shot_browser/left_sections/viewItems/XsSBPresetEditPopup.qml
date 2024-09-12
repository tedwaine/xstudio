// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.14

import xStudio 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

XsWindow{
    id: presetEditPopup

    property var presetIndex: null
    property string entityType: "Versions"
    property string entityName: ""
    property string entityCategory: "Group"

    property int itemHeight: 24

    property int dragWidth: btnWidth/2
    property int enableWidth: itemHeight
    property int termWidth: btnWidth*4.2
    property int modeWidth: itemHeight
    property int closeWidth: btnWidth

    width: 460
    height: 40 + 200 + (nameDiv.height + coln.spacing*2) //+ presetList.height
    // transientParent: parent
    // remove focus from text widgets.
    onClosing: coln.focus = true

    onPresetIndexChanged: {
        presetTermModel.rootIndex = presetIndex
        // presetTermModel.model = ShotBrowserEngine.presetsModel
        presetDelegateModel.newTermParent = presetIndex
    }

    QTreeModelToTableModel {
        id: presetTermModel
        model: ShotBrowserEngine.presetsModel
    }

    DelegateModel {
        id: presetDelegateModel
        property var notifyModel: presetTermModel
        model: notifyModel

        property var newTermParent: null

        delegate: XsSBPresetEditItem{
            width: presetList.width
            height: itemHeight
            termModel: ShotBrowserEngine.presetsModel.termLists[entityType]
            delegateModel: presetDelegateModel
        }
    }


    ColumnLayout { id: coln
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Item{ id: nameDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight*1.2

            RowLayout {
                width: parent.width
                height: parent.height
                spacing: 1

                Item{
                    Layout.preferredWidth: dragWidth
                    Layout.fillHeight: true
                }
                Item {
                    Layout.preferredWidth: enableWidth
                    Layout.fillHeight: true
                }
                XsText{
                    Layout.preferredWidth: termWidth
                    Layout.fillHeight: true
                    text: entityCategory+" name:"
                }
                Item {
                    Layout.preferredWidth: modeWidth
                    Layout.fillHeight: true
                }
                XsTextField{ id: nameEditDiv
                    Layout.fillWidth: true
                    Layout.preferredWidth: btnWidth*4.2
                    Layout.fillHeight: true
                    text: entityName
                    placeholderText: entityName
                    onEditingFinished: {
                        if(ShotBrowserEngine.presetsModel.get(presetIndex.parent,"typeRole") == "presets")
                            ShotBrowserEngine.presetsModel.set(presetIndex, text, "nameRole")
                        else
                            ShotBrowserEngine.presetsModel.set(presetIndex.parent, text, "nameRole")
                    }

                    background:
                    Rectangle{
                        color: nameEditDiv.activeFocus? Qt.darker(palette.highlight, 1.5): nameEditDiv.hovered? Qt.lighter(palette.base, 2):Qt.lighter(palette.base, 1.5)
                        border.width: nameEditDiv.hovered || nameEditDiv.active? 1:0
                        border.color: palette.highlight
                        opacity: enabled? 0.7 : 0.3
                    }
                }
                Item{
                    Layout.preferredWidth: closeWidth + itemHeight + 2
                    Layout.fillHeight: true
                }

            }
        }

        XsListView{ id: presetList
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: presetDelegateModel
            interactive: true
            spacing: 1

            footer: Item{
                    width: presetList.width
                    height: itemHeight
                    XsSBPresetEditNewItem{
                        anchors.fill: parent
                        anchors.topMargin: 1
                        delegateModel: presetDelegateModel
                        termModel: ShotBrowserEngine.presetsModel.termLists[entityType]
                }
            }
        }

        RowLayout{
            Layout.fillWidth: true
            Layout.maximumHeight: itemHeight
            Layout.minimumHeight: itemHeight

            Item{
                Layout.preferredWidth: parent.width / 3 * 2
                Layout.fillHeight: true
            }

            XsPrimaryButton{
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "Close"
                onClicked: {
                    close()
                }
            }
        }
    }
}

