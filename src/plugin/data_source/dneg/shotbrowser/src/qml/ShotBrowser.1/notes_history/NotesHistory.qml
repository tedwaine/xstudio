// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQuick.Dialogs 1.3
import QtGraphicalEffects 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QuickFuture 1.0
import QuickPromise 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import ShotBrowser 1.0

Item{
    id: panel
    anchors.fill: parent

    property bool isPanelEnabled: true
    property var dataModel: results

    property real panelPadding: XsStyleSheet.panelPadding
    property color panelColor: XsStyleSheet.panelBgColor

    property real buttonHeight: XsStyleSheet.widgetStdHeight

    property var activeScopeIndex: ShotBrowserEngine.presetsModel.index(-1,-1)
    property var activeTypeIndex: ShotBrowserEngine.presetsModel.index(-1,-1)

    // Track the uuid of the media that is currently visible in the Viewport
    property var onScreenMediaUuid: currentPlayhead.mediaUuid
    property var onScreenLogicalFrame: currentPlayhead.logicalFrame

    readonly property string panelType: "NotesHistory"

    property int queryCounter: 0
    property int queryRunning: 0

    property bool isPaused: false

    onOnScreenMediaUuidChanged: {if(visible) updateTimer.start()}

    onOnScreenLogicalFrameChanged: {
        if(updateTimer.running) {
            updateTimer.restart()
            if(isPanelEnabled && !isPaused) {
                isPaused = true
                resultsSelectionModel.clear()
                results.setResultData([])
                ShotBrowserEngine.liveLinkKey = ""
                ShotBrowserEngine.liveLinkMetadata = "null"
            }
        }
    }

    Timer {
        id: updateTimer
        interval: 500
        running: false
        repeat: false
        onTriggered: {
            isPaused = false
            ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid)
        }
    }

    ShotBrowserResultModel {
        id: results
    }

    ItemSelectionModel {
        id: resultsSelectionModel
        model: results
    }

    Connections {
        target: ShotBrowserEngine
        function onReadyChanged() {
            setIndexesFromPreferences()
        }
    }

    function setIndexesFromPreferences() {
        if(ShotBrowserEngine.ready) {
            prefs.updateFromValue()
            if(! activeScopeIndex.valid && (noteScopePref.value || prefs.scope)) {
                if(prefs.scope) {
                    let i = getScopeIndex(prefs.scope)
                    if(i.valid && activeScopeIndex != i)
                        activeScopeIndex = i
                }

                if(!activeScopeIndex.valid) {
                    let i = getScopeIndex(noteScopePref.value)
                    if(i.valid && activeScopeIndex != i)
                        activeScopeIndex = i
                }
            }

            if(! activeTypeIndex.valid && (noteTypePref.value || prefs.type)){
                if(prefs.type) {
                    let i = getTypeIndex(prefs.type)
                    if(i.valid && activeTypeIndex != i)
                        activeTypeIndex = i
                }
                if(!activeTypeIndex.valid) {
                    let i = getTypeIndex(noteTypePref.value)
                    if(i.valid && activeTypeIndex != i)
                        activeTypeIndex = i
                }
            }
        }
    }

    // get presets node under group
    function getScopeIndex(scope_name) {
        let m = ShotBrowserEngine.presetsModel
        let p = m.searchRecursive("aac8207e-129d-4988-9e05-b59f75ae2f75", "idRole", ShotBrowserEngine.presetsModel.index(-1, -1), 0, 1)
        return m.searchRecursive(scope_name, "nameRole", p, 0, 1)
    }

    function getTypeIndex(type_name) {
        let m = ShotBrowserEngine.presetsModel
        let p = m.searchRecursive("aac8207e-129d-4988-9e05-b59f75ae2f75", "idRole", ShotBrowserEngine.presetsModel.index(-1, -1), 0, 1)
        return m.searchRecursive(type_name, "nameRole", p, 0, 1)
    }

    XsModelProperty {
        id: prefs
        index: panels_layout_model_index
        role: "user_data"

        property string scope: ""
        property string type: ""

        onTypeChanged: {
            let i = createDefaults()
            if(i["type"] != type) {
                i["type"] = type
                value = i
            }
        }

        onScopeChanged: {
            let i = createDefaults()
            if(i["scope"] != scope) {
                i["scope"] = scope
                value = i
            }
        }

        onValueChanged: updateFromValue()

        function createDefaults() {
            let i = {}
            i["type"] = value != undefined && value.hasOwnProperty("type") ? value["type"] : ""
            i["scope"] = value != undefined && value.hasOwnProperty("scope") ? value["scope"] : ""
            return i
        }

        function updateFromValue() {
            if(value) {
                if(value["scope"] && scope != value["scope"])
                    scope = value["scope"]
                if(value["type"] && type != value["type"])
                    type = value["type"]
            }
        }
    }

    XsPreference {
        id: noteScopePref
        path: "/plugin/data_source/shotbrowser/note_history/scope"
    }

    XsPreference {
        id: noteTypePref
        path: "/plugin/data_source/shotbrowser/note_history/type"
    }

    onActiveScopeIndexChanged: {
        if(activeScopeIndex && activeScopeIndex.valid) {
            let m = activeScopeIndex.model
            let i = m.get(activeScopeIndex, "nameRole")
            noteScopePref.value = i
            prefs.scope = i
        }
    }

    onActiveTypeIndexChanged: {
        if(activeTypeIndex && activeTypeIndex.valid) {
            let m = activeTypeIndex.model
            let i = m.get(activeTypeIndex, "nameRole")
            noteTypePref.value = i
            prefs.type = i
        }
    }



    Connections {
        target: ShotBrowserEngine
        function onLiveLinkMetadataChanged() {
            if(!isPaused && isPanelEnabled && panel.visible) {
                runQuery()
            }
        }
    }

    onIsPanelEnabledChanged: {
        if(isPanelEnabled) {
            if(!ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid))
                runQuery()
        }
    }

    function runQuery() {
        if(isPanelEnabled && !isPaused && activeScopeIndex.valid && activeTypeIndex.valid) {

            // make sure the results appear in sync.
            queryCounter += 1
            queryRunning += 1
            let i = queryCounter

            Future.promise(
                ShotBrowserEngine.executeQuery(
                        [
                            ShotBrowserEngine.presetsModel.get(
                                activeScopeIndex,
                                "jsonPathRole"
                            ),
                            ShotBrowserEngine.presetsModel.get(
                                activeTypeIndex,
                                "jsonPathRole"
                            )
                        ]
                    )
                ).then(function(json_string) {
                    if(queryCounter == i) {
                        resultsSelectionModel.clear()
                        results.setResultData([json_string])
                    }
                    queryRunning -= 1

                },
                function() {
                    resultsSelectionModel.clear()
                    results.setResultData([])
                    queryRunning -= 1
                })
        }
    }

    function activateScope(clickedIndex){
        if(clickedIndex.valid) {
            activeScopeIndex = clickedIndex
            runQuery()
        }
    }

    function activateType(clickedIndex){
        if(clickedIndex.valid) {
            activeTypeIndex = clickedIndex
            runQuery()
        }
    }

    Component.onCompleted: {
        if(visible) {
            ShotBrowserEngine.connected = true
            if(!ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid))
                runQuery()
        }
        setIndexesFromPreferences()
    }

    onVisibleChanged: {
        if(visible) {
            ShotBrowserEngine.connected = true
            if(!ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid))
                runQuery()
        }
    }

    XsGradientRectangle{ id: backgroundDiv
        anchors.fill: parent
    }

    NotesHistoryResultPopup {
        id: resultPopup
        menu_model_name: "note_history_popup"
        popupSelectionModel: resultsSelectionModel
    }


    ColumnLayout{
        anchors.fill: parent
        spacing: 0

        NotesHistoryTitleDiv{id: titleDiv
            titleButtonHeight: (buttonHeight + 4)
            Layout.fillWidth: true
            Layout.preferredHeight: titleButtonHeight*2 + (panelPadding*2) + titleButtonSpacing
        }

        NotesHistoryListDiv{ id: contentDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        NotesHistoryActionDiv{id: buttonsDiv
            Layout.fillWidth: true
            Layout.preferredHeight: buttonHeight + (panelPadding*2)
        }

    }
}