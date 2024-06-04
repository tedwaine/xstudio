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
import xstudio.qml.bookmarks 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.clipboard 1.0
import ShotBrowser 1.0

Item{
    id: panel
    anchors.fill: parent

    property string resultViewTitle: ""

    property alias currentCategory: prefs.category
    property bool isGroupedByLatest: false

    property real buttonSpacing: 1
    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight + 4
    property real panelPadding: XsStyleSheet.panelPadding
    property color panelColor: XsStyleSheet.panelBgColor

    property bool queryRunning: queryRunningCount > 0

    property var onScreenMediaUuid: currentPlayhead.mediaUuid
    property var onScreenLogicalFrame: currentPlayhead.logicalFrame

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

    readonly property string panelType: "ShotBrowser"

    property string onDisk: ""

    property int queryCounter: 0
    property int queryRunningCount: 0

    property bool isPaused: false

    onOnScreenMediaUuidChanged: {if(visible) updateTimer.start()}

    onOnScreenLogicalFrameChanged: {
        if(updateTimer.running)
            updateTimer.restart()
        if(!isPaused && (currentCategory == "Menus" && currentPresetIndex.valid) || (currentCategory == "Tree" && sequenceTreeLiveLink)) {
            isPaused = true
            resultsSelectionModel.clear()
            resultsBaseModel.setResultData([])
            ShotBrowserEngine.liveLinkKey = ""
            ShotBrowserEngine.liveLinkMetadata = "null"
        }
    }

    Timer {
        id: updateTimer
        interval: 500
        running: false
        repeat: false
        onTriggered: {
            isPaused = false
            updateMetaData()
        }
    }

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
            if(panel.visible && !isPaused) {
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
                prefs.updateFromValue()
                if(projectIndex == null) {
                    if(prefs.project) {
                        let i = getProjectIndexFromName(prefs.project)
                        if(i && i.valid)
                            projectIndex = i
                    }
                    if(projectIndex == null)
                        projectIndex = getProjectIndexFromId(projectPref.value)
                }
                // console.log("onReadyChanged", projectIndex, prefs.project, projectPref.value)
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
    }

    XsModelProperty {
        id: prefs
        index: panels_layout_model_index
        role: "user_data"

        property int treeWidth: 200
        property int presetWidth: 200
        property string category: "Tree"
        property string project: "NSFL"

        onTreeWidthChanged: {
            let i = createDefaults()
            if(i["tree_width"] != treeWidth) {
                i["tree_width"] = treeWidth
                value = i
            }
        }

        onPresetWidthChanged: {
            let i = createDefaults()
            if(i["preset_width"] != presetWidth) {
                i["preset_width"] = presetWidth
                value = i
            }
        }

        onCategoryChanged: {
            let i = createDefaults()
            if(i["category"] != category) {
                i["category"] = category
                value = i
            }
        }

        onProjectChanged: {
            let i = createDefaults()
            if(i["project"] != project) {
                i["project"] = project
                value = i
            }

            if(ShotBrowserEngine.ready) {
                let index = getProjectIndexFromName(project)
                if(index.valid && projectIndex != index) {
                    projectIndex = index
                    // console.log("onProjectChanged", projectIndex, project)
                }
            }
        }

        function createDefaults() {
            let i = {}
            i["tree_width"] = value != undefined && value.hasOwnProperty("tree_width") ? value["tree_width"] : 200
            i["preset_width"] = value != undefined && value.hasOwnProperty("preset_width") ? value["preset_width"] : 200
            i["category"] = value != undefined && value.hasOwnProperty("category") ? value["category"] : "Tree"
            i["project"] = value != undefined && value.hasOwnProperty("project") ? value["project"] : "NSFL"
            return i
        }

        onValueChanged: updateFromValue()

        function updateFromValue() {
            if(value) {
                if(value["preset_width"] && presetWidth != value["preset_width"])
                    presetWidth = value["preset_width"]
                if(value["tree_width"] && treeWidth != value["tree_width"])
                    treeWidth = value["tree_width"]
                if(value["category"] && category != value["category"])
                    category = value["category"]
                if(value["project"] && project != value["project"])
                    project = value["project"]
            }
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
        model: results
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
            ShotBrowserEngine.cacheProject(i)
            sequenceModel = ShotBrowserEngine.sequenceTreeFilterModel(i)

            if(projectPref.value != i)
                projectPref.value = i

            prefs.project = m.get(projectIndex, "nameRole")

            // console.log("onProjectIndexChanged", projectPref.value, prefs.project)
        }
    }

    onCurrentCategoryChanged: leftSection.SplitView.preferredWidth = currentCategory == "Tree" ? prefs.treeWidth + prefs.presetWidth : prefs.presetWidth

    XsSplitView {
        id: main_split
        anchors.fill: parent

        XsSBLeftSection{ id: leftSection
            SplitView.preferredWidth: currentCategory == "Tree" ? prefs.treeWidth + prefs.presetWidth : prefs.presetWidth
            SplitView.fillHeight: true
            onWidthChanged: {
                if(SplitView.view.resizing) {
                    if(currentCategory == "Tree")
                        prefs.presetWidth = width - prefs.treeWidth
                    else
                        prefs.presetWidth = width
                }
            }
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
                queryRunningCount += 1
                let i = queryCounter

                Future.promise(
                    ShotBrowserEngine.executeQuery(
                        [ShotBrowserEngine.presetsModel.get(currentPresetIndex, "jsonPathRole")], {}, [])
                    ).then(function(json_string) {
                        // console.log(json_string)
                        if(queryCounter == i) {
                            resultsSelectionModel.clear()
                            resultsBaseModel.setResultData([json_string])
                        }
                        queryRunningCount -= 1
                    },
                    function() {
                        resultsBaseModel.setResultData([])
                        queryRunningCount -= 1
                    })
            } else if(currentCategory == "Recent") {
                queryCounter += 1
                queryRunningCount += 1

                let i = queryCounter
                let pi = ShotBrowserEngine.presetsModel.termModel("Project").get(projectIndex, "idRole")

                Future.promise(
                    ShotBrowserEngine.executeProjectQuery(
                        [ShotBrowserEngine.presetsModel.get(currentPresetIndex, "jsonPathRole")], pi, {}, [])
                    ).then(function(json_string) {
                        // console.log(json_string)
                        if(queryCounter == i) {
                            resultsSelectionModel.clear()
                            resultsBaseModel.setResultData([json_string])
                        }
                        queryRunningCount -= 1
                    },
                    function() {
                        resultsBaseModel.setResultData([])
                        queryRunningCount -= 1
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
                    queryRunningCount += 1

                    let i = queryCounter
                    Future.promise(
                        ShotBrowserEngine.executeProjectQuery(
                            [ShotBrowserEngine.presetsModel.get(currentPresetIndex, "jsonPathRole")], pi, {}, custom)
                        ).then(function(json_string) {
                            // console.log(json_string)
                            if(queryCounter == i) {
                                resultsSelectionModel.clear()
                                resultsBaseModel.setResultData([json_string])
                            }
                            queryRunningCount -= 1
                        },
                        function() {
                            resultsBaseModel.setResultData([])
                            queryRunningCount -= 1
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
        prefs.updateFromValue()
    }

    onVisibleChanged: {
        if(visible) {
            ShotBrowserEngine.connected = true
            updateMetaData()
        }
    }

}