// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import QuickFuture 1.0
import QuickPromise 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

Rectangle{
    color: "transparent" //panelColor

    property real itemHeight: XsStyleSheet.widgetStdHeight

    Component.onCompleted: {
        populateModels()
    }

    Connections {
        target: ShotBrowserEngine
        function onReadyChanged() {
            populateModels()
        }
    }

    function populateModels() {
        if(ShotBrowserEngine.ready && ShotBrowserEngine.presetsModel.rowCount()) {
            quickModel.rootIndex = ShotBrowserEngine.presetsModel.searchRecursive("137aa66a-87e2-4c53-b304-44bd7ff9f755", "idRole")
            quickCombo.currentIndex = 0
        }
    }

    function executeQuery(queryIndex, action) {
        if(queryIndex.valid) {
            // clear current result set
            quickResults.setResultData([])

            let pi = ShotBrowserEngine.presetsModel.termModel("Project").get(projectIndex, "idRole")
            let custom = []
            let seqsel = sequenceSelectionModel.selectedIndexes
            for(let i=0;i<seqsel.length;i++) {
                let t = seqsel[i].model.get(seqsel[i],"typeRole")
                if(t == "Shot") {
                    custom.push({
                        "enabled": true,
                        "type": "term",
                        "term": "Shot",
                        "value": seqsel[i].model.get(seqsel[i],"nameRole")
                    })
                } else if( t == "Sequence") {
                    custom.push({
                        "enabled": true,
                        "type": "term",
                        "term": "Sequence",
                        "value": seqsel[i].model.get(seqsel[i],"nameRole")
                    })
                }
            }


            // only run, if selection in tree.
            if(custom.length) {

                let result_json = []
                let result_count = custom.length

                for(let i = 0; i< result_count;i++) {
                    Future.promise(
                        ShotBrowserEngine.executeProjectQuery(
                            [ShotBrowserEngine.presetsModel.get(queryIndex, "jsonPathRole")], pi, {}, [custom[i]])
                        ).then(function(json_string) {
                            result_json[i] = json_string
                            result_count -= 1
                            if(!result_count) {
                                quickResults.setResultData(result_json)
                                if(action == "playlist") {
                                    let indexes = []
                                    for(let i=0;i<quickResults.rowCount();i++)
                                        indexes.push(quickResults.index(i,0))
                                    ShotBrowserHelpers.addToCurrent(indexes, false)
                                }
                            }
                        },
                        function() {
                            result_json[i] = ""
                            result_count -= 1
                            if(!result_count) {
                                quickResults.setResultData(result_json)
                                if(action == "playlist") {
                                    let indexes = []
                                    for(let i=0;i<quickResults.rowCount();i++)
                                        indexes.push(quickResults.index(i,0))
                                    ShotBrowserHelpers.addToCurrent(indexes, false)
                                }
                            }
                        })
                    }
            }
        }
    }

    ShotBrowserResultModel {
        id: quickResults
    }
    DelegateModel {
        id: quickModel
        property var notifyModel: ShotBrowserEngine.presetsModel
        rootIndex: ShotBrowserEngine.presetsModel.index(-1,-1)
        model: notifyModel
        delegate:  quickCombo.delegate
    }

    RowLayout {
        width: parent.width - buttonSpacing*2
        height: parent.height - buttonSpacing*2
        anchors.centerIn: parent
        spacing: buttonSpacing*4

        XsText{
            Layout.fillWidth: true
            Layout.minimumWidth: 20
            Layout.preferredWidth: 100
            Layout.maximumWidth: 120
            Layout.fillHeight: true
            text: "Quick Load:"
            elide: Text.ElideRight
        }

        Item{
            Layout.fillWidth: true
            Layout.minimumWidth: 120
            Layout.preferredWidth: 120
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: buttonSpacing

                Item{
                    Layout.fillWidth: true
                    Layout.preferredHeight: itemHeight

                    XsComboBox {
                        id: quickCombo
                        model: quickModel
                        textRole: "nameRole"
                        currentIndex: 0
                        width: parent.width
                        height: itemHeight
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }



                Item{
                    Layout.fillWidth: true
                    Layout.preferredHeight: itemHeight

                RowLayout {
                    anchors.fill: parent
                    spacing: buttonSpacing

                    XsPrimaryButton{
                        Layout.preferredWidth: parent.width/2
                        Layout.preferredHeight: itemHeight
                        imgSrc: ""
                        text: "Add"
                        onClicked: {
                            if(quickCombo.currentIndex != -1)
                                executeQuery(quickModel.modelIndex(quickCombo.currentIndex), "playlist")
                        }
                    }

                    XsPrimaryButton{
                        Layout.preferredWidth: parent.width/2
                        Layout.preferredHeight: itemHeight
                        imgSrc: ""
                        text: "View In Cut"
                    }
                }
                }

            }
        }


    }

}