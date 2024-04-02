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
import xstudio.qml.module 1.0
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
    property var onScreenMediaUuid: appWindow.onScreenMediaUuid

    property int queryCounter: 0

    onOnScreenMediaUuidChanged: {if(visible) ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid)}

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
            if(ShotBrowserEngine.ready) {
                if(! activeScopeIndex.valid && noteScopePref.value) {
                    let i = getScopeIndex(noteScopePref.value)
                    if(i && i.valid && activeScopeIndex != i)
                        activeScopeIndex = i
                }
                if(! activeTypeIndex.valid && noteTypePref.value) {
                    let i = getTypeIndex(noteTypePref.value)
                    if(i && i.valid && activeTypeIndex != i)
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


    XsPreference {
        id: noteScopePref
        path: "/plugin/data_source/shotbrowser/note_history/scope"

        onValueChanged: {
            let i = getScopeIndex(value)
            if(i && i.valid && activeScopeIndex != i)
                activeScopeIndex = i
        }
    }

    XsPreference {
        id: noteTypePref
        path: "/plugin/data_source/shotbrowser/note_history/type"

        onValueChanged: {
            let i = getTypeIndex(value)
            if(i && i.valid && activeTypeIndex != i)
                activeTypeIndex = i
        }
    }

    onActiveScopeIndexChanged: {
        if(activeScopeIndex && activeScopeIndex.valid) {
            let m = activeScopeIndex.model
            let i = m.get(activeScopeIndex, "nameRole")
            noteScopePref.value = i
        }
    }

    onActiveTypeIndexChanged: {
        if(activeTypeIndex && activeTypeIndex.valid) {
            let m = activeTypeIndex.model
            let i = m.get(activeTypeIndex, "nameRole")
            noteTypePref.value = i
        }
    }



    Connections {
        target: ShotBrowserEngine
        function onLiveLinkMetadataChanged() {
            if(isPanelEnabled && panel.visible) {
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
        if(isPanelEnabled && activeScopeIndex.valid && activeTypeIndex.valid) {

            // make sure the results appear in sync.
            queryCounter += 1
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
                },
                function() {
                    resultsSelectionModel.clear()
                    results.setResultData([])
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