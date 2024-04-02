// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import QuickFuture 1.0
import QuickPromise 1.0

import xStudioReskin 1.0
import xstudio.qml.module 1.0
import xstudio.qml.bookmarks 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.clipboard 1.0
import ShotBrowser 1.0

Item{
    id: panel
    anchors.fill: parent

    property bool isTestMode: false
    property string resultViewTitle: ""

    property alias currentCategory: categoryPref.value
    property bool isGroupedByLatest: false

    property real buttonSpacing: 1
    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight + 4
    property real panelPadding: XsStyleSheet.panelPadding
    property color panelColor: XsStyleSheet.panelBgColor

    property bool queryRunning: false

    property var onScreenMediaUuid: appWindow.onScreenMediaUuid

    property var projectIndex: null
    property var sequenceModel: null
    property var currentPresetIndex: ShotBrowserEngine.presetsModel.index(-1,-1)

    property bool sequenceTreeLiveLink: false

    property alias sortByNaturalOrder: resultsFilteredModel.sortByNaturalOrder
    property alias sortByCreationDate: resultsFilteredModel.sortByCreationDate
    property alias sortByShotName: resultsFilteredModel.sortByShotName
    property alias sortInAscending: resultsFilteredModel.sortInAscending
    property alias pipeStep: resultsFilteredModel.filterPipeStep
    property alias nameFilter: resultsFilteredModel.filterName

    property string onDisk: ""
    property int queryCounter: 0

    onOnScreenMediaUuidChanged: updateMetaData()

    Clipboard {
        id: clipboard
    }

    function updateMetaData() {
        if(visible) {
            if((currentCategory == "Menus" && currentPresetIndex.valid) || (currentCategory == "Tree" && sequenceTreeLiveLink))
                return ShotBrowserHelpers.updateMetadata(true, onScreenMediaUuid)
        }
        return false
    }

    function updateSequenceSelection() {
        if(visible) {
            if(currentCategory == "Tree" && sequenceTreeLiveLink) {
                // update tree selection
                let pname = ShotBrowserEngine.getProjectFromMetadata()
                let sname = ShotBrowserEngine.getShotSequenceFromMetadata()
                if(!projectIndex.valid || projectIndex.model.get(projectIndex,"nameRole") != pname) {
                    projectIndex = getProjectIndexFromName(pname)
                }

                let si = sequenceModel.searchRecursive(sname)
                if(si.valid)
                    sequenceSelectionModel.select(si, ItemSelectionModel.ClearAndSelect)
            }
        }
    }

    onSequenceTreeLiveLinkChanged: {
        if(sequenceTreeLiveLink) {
            updateMetaData()
            updateSequenceSelection()
        }
    }

    Connections {
        target: ShotBrowserEngine
        function onLiveLinkMetadataChanged() {
            if(panel.visible) {
                updateSequenceSelection()

                if(currentCategory == "Menus" && currentPresetIndex.valid) {
                    executeQuery()
                }
            }
        }
    }

    Connections {
        target: ShotBrowserEngine
        function onReadyChanged() {
            if(ShotBrowserEngine.ready) {
                if(projectIndex == null) {
                    projectIndex = getProjectIndexFromId(projectPref.value)
                }
                ShotBrowserEngine.presetModels.tree.showHidden = showHiddenPref.value
                ShotBrowserEngine.presetModels.recent.showHidden = showHiddenPref.value
                ShotBrowserEngine.presetModels.menus.showHidden = showHiddenPref.value
            }
        }
    }

    Connections {
        target: ShotBrowserEngine.presetsModel
        function onPresetChanged(index) {
            if(currentPresetIndex == index) {
                executeQuery()
            }
        }
    }

    function getProjectIndexFromId(project_id) {
        let m = ShotBrowserEngine.presetsModel.termModel("Project")
        return m.searchRecursive(project_id, "idRole")
    }

    function getProjectIndexFromName(project_name) {
        let m = ShotBrowserEngine.presetsModel.termModel("Project")
        return m.searchRecursive(project_name, "nameRole")
    }

    XsPreference {
        id: projectPref
        path: "/plugin/data_source/shotbrowser/browser/project_id"

        onValueChanged: {
            let i = getProjectIndexFromId(value)
            if(ShotBrowserEngine.ready && i && i.valid && projectIndex != i)
                projectIndex = i
        }
    }

    XsPreference {
        id: categoryPref
        path: "/plugin/data_source/shotbrowser/browser/category"
    }

    // XsPreference {
    //     id: categoryWidthPref
    //     path: "/plugin/data_source/shotbrowser/browser/category_width"
    //     onValueChanged: {
    //         ShotBrowserEngine.presetModels.tree.category_width = value
    //         ShotBrowserEngine.presetModels.recent.category_width = value
    //         ShotBrowserEngine.presetModels.menus.category_width = value
    //     }
    // }

    XsPreference {
        id: showHiddenPref
        path: "/plugin/data_source/shotbrowser/browser/show_hidden"
        onValueChanged: {
            ShotBrowserEngine.presetModels.tree.showHidden = value
            ShotBrowserEngine.presetModels.recent.showHidden = value
            ShotBrowserEngine.presetModels.menus.showHidden = value
        }
    }

    ItemSelectionModel {
        id: sequenceExpandedModel
        model: sequenceModel
    }

    ItemSelectionModel {
        id: sequenceSelectionModel
        model: sequenceModel
        onSelectionChanged: {
            // check parents are expanded..
            // build list of parents..
            sequenceExpandedModel.select(
                helpers.createItemSelection(
                    helpers.getParentIndexesFromRange(selected)
                ),
                ItemSelectionModel.Select
            )
            executeQuery()
        }
    }

    ItemSelectionModel {
        id: presetsExpandedModel
        model: ShotBrowserEngine.presetsModel
    }

    ItemSelectionModel {
        id: presetsSelectionModel
        model: ShotBrowserEngine.presetsModel
    }

    ShotBrowserResultModel {
        id: resultsBaseModel
        isGrouped: true
    }

    ShotBrowserResultFilterModel {
        id: resultsFilteredModel
        sourceModel: resultsBaseModel
    }

    QTreeModelToTableModel {
        id: results
        model: resultsFilteredModel
    }

    ItemSelectionModel {
        id: resultsSelectionModel
        model: resultsBaseModel
    }

    ShotHistoryResultPopup {
        id: versionResultPopup
        menu_model_name: "version_shot_browser_popup"
        popupSelectionModel: resultsSelectionModel
    }

    NotesHistoryResultPopup {
        id: noteResultPopup
        menu_model_name: "note_shot_browser_popup"
        popupSelectionModel: resultsSelectionModel
    }

    XsSBRPlaylistResultPopup {
        id: playlistResultPopup
        menu_model_name: "playlist_shot_browser_popup"
        popupSelectionModel: resultsSelectionModel
    }

    onProjectIndexChanged: {
        if(projectIndex && projectIndex.valid) {
            let m = ShotBrowserEngine.presetsModel.termModel("Project")
            let i = m.get(projectIndex, "idRole")
            projectPref.value = i
            ShotBrowserEngine.cacheProject(i)
            sequenceModel = ShotBrowserEngine.sequenceTreeFilterModel(i)
        }
    }

    XsSplitView {
        anchors.fill: parent

        XsSBLeftSection{ id: leftSection
            SplitView.preferredWidth: visibleWidth
            SplitView.fillHeight: true
        }
        XsSBRightSection{
            SplitView.fillWidth: true
            SplitView.fillHeight: true
        }

    }

    onOnDiskChanged: {
        resultsFilteredModel.filterChn = (onDisk == "chn")
        resultsFilteredModel.filterLon = (onDisk == "lon")
        resultsFilteredModel.filterMtl = (onDisk == "mtl")
        resultsFilteredModel.filterMum = (onDisk == "mum")
        resultsFilteredModel.filterVan = (onDisk == "van")
        resultsFilteredModel.filterSyd = (onDisk == "syd")
    }

    function executeQuery() {
        if(currentPresetIndex && currentPresetIndex.valid) {

            nameFilter = ""
            pipeStep = ""
            // onDisk = ""

            resultsSelectionModel.clear()

            // pipeStep
            if(currentCategory == "Menus") {
                queryCounter += 1
                let i = queryCounter
                queryRunning = true

                Future.promise(
                    ShotBrowserEngine.executeQuery(
                        [ShotBrowserEngine.presetsModel.get(currentPresetIndex, "jsonPathRole")], {}, [])
                    ).then(function(json_string) {
                        // console.log(json_string)
                        if(queryCounter == i) {
                            resultsSelectionModel.clear()
                            resultsBaseModel.setResultData([json_string])
                            queryRunning = false
                        }
                    },
                    function() {
                        resultsBaseModel.setResultData([])
                    })
            } else if(currentCategory == "Recent") {
                queryCounter += 1
                let i = queryCounter
                let pi = ShotBrowserEngine.presetsModel.termModel("Project").get(projectIndex, "idRole")
                queryRunning = true

                Future.promise(
                    ShotBrowserEngine.executeProjectQuery(
                        [ShotBrowserEngine.presetsModel.get(currentPresetIndex, "jsonPathRole")], pi, {}, [])
                    ).then(function(json_string) {
                        // console.log(json_string)
                        if(queryCounter == i) {
                            resultsSelectionModel.clear()
                            resultsBaseModel.setResultData([json_string])
                            queryRunning = false
                        }
                    },
                    function() {
                        resultsBaseModel.setResultData([])
                    })
            } else {
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
                if(custom.length || ShotBrowserEngine.presetsModel.get(currentPresetIndex, "entityRole") == "Playlists") {
                    queryCounter += 1
                    let i = queryCounter
                    queryRunning = true
                    Future.promise(
                        ShotBrowserEngine.executeProjectQuery(
                            [ShotBrowserEngine.presetsModel.get(currentPresetIndex, "jsonPathRole")], pi, {}, custom)
                        ).then(function(json_string) {
                            // console.log(json_string)
                            if(queryCounter == i) {
                                resultsSelectionModel.clear()
                                resultsBaseModel.setResultData([json_string])
                                queryRunning = false
                            }
                        },
                        function() {
                            resultsBaseModel.setResultData([])
                        })
                } else {
                    resultsBaseModel.setResultData([])
                }
            }
        }
    }

    function activatePreset(clickedIndex){
        // map to preset model..
        currentPresetIndex = clickedIndex
        if(projectIndex && projectIndex.valid) {
            if(currentPresetIndex.valid) {
                executeQuery()
            }
        }
    }

    Component.onCompleted: {
        if(visible) {
            ShotBrowserEngine.connected = true
        }
    }

    onVisibleChanged: {
        if(visible) {
            ShotBrowserEngine.connected = true
            updateMetaData()
        }
    }

}