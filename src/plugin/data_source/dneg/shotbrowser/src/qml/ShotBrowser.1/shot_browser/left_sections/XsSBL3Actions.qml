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
            quickModel.rootIndex = quickModel.notifyModel.mapFromSource(ShotBrowserEngine.presetsModel.searchRecursive("137aa66a-87e2-4c53-b304-44bd7ff9f755", "idRole"))
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
                                let indexes = []
                                for(let j=0;j<quickResults.rowCount();j++) {
                                    indexes.push(quickResults.index(j,0))
                                }
                                if(action == "playlist") {
                                    ShotBrowserHelpers.addToCurrent(indexes, false)
                                } else if(action == "sequence") {
                                    let seq_map = {}

                                    for(let j=0;j<indexes.length;j++) {
                                        let seq = quickResults.get(indexes[j], "sequenceRole")
                                        if(seq_map[seq] === undefined)
                                            seq_map[seq] = [indexes[j]]
                                        else
                                            seq_map[seq].push(indexes[j])
                                    }
                                    for(let key in seq_map) {
                                        ShotBrowserHelpers.addToPlaylist(seq_map[key], null, null, key, ShotBrowserHelpers.conformToNewSequenceCallback)
                                    }
                                }
                            }
                        },
                        function() {
                            result_json[i] = ""
                            result_count -= 1
                            if(!result_count) {
                                quickResults.setResultData(result_json)
                                let indexes = []
                                for(let j=0;j<quickResults.rowCount();j++)
                                    indexes.push(quickResults.index(j,0))

                                if(action == "playlist") {
                                    ShotBrowserHelpers.addToCurrent(indexes, false)
                                } else if(action == "sequence") {

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

    ShotBrowserPresetFilterModel {
        id: filterModel
        showHidden: false
        onlyShowFavourite: true
        sourceModel: ShotBrowserEngine.presetsModel
    }

    DelegateModel {
        id: quickModel
        property var notifyModel: filterModel
        rootIndex: filterModel.index(-1,-1)
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
                        onActivated: prefs.quickLoad = currentText
                        onCountChanged: currentIndex = find(prefs.quickLoad)
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
                            text: "Add"
                            onClicked: {
                                if(quickCombo.currentIndex != -1)
                                    executeQuery(
                                        quickModel.notifyModel.mapToSource(quickModel.modelIndex(quickCombo.currentIndex)),
                                         "playlist"
                                    )
                            }
                        }

                        XsPrimaryButton{
                            Layout.preferredWidth: parent.width/2
                            Layout.preferredHeight: itemHeight
                            text: "Conform To New Sequence"
                            onClicked: {
                                if(quickCombo.currentIndex != -1)
                                    executeQuery(
                                        quickModel.notifyModel.mapToSource(quickModel.modelIndex(quickCombo.currentIndex)),
                                         "sequence"
                                    )
                            }
                        }
                    }
                }
            }
        }
    }
}