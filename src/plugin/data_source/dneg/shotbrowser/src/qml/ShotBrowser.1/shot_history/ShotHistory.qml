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

    property bool isTestMode: false

    property bool isPanelEnabled: true
    property var dataModel: results

    property real panelPadding: XsStyleSheet.panelPadding
    property color panelColor: XsStyleSheet.panelBgColor

    property real buttonHeight: XsStyleSheet.widgetStdHeight

    property var activeScopeIndex: ShotBrowserEngine.presetsModel.index(-1,-1)

    // Track the uuid of the media that is currently visible in the Viewport
    property var onScreenMediaUuid: appWindow.onScreenMediaUuid

    property int queryCounter: 0

    property real btnHeight: XsStyleSheet.widgetStdHeight + 4

    onOnScreenMediaUuidChanged: {if(visible) ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid)}

    Connections {
        target: ShotBrowserEngine
        function onReadyChanged() {
            setIndexFromPreference()
        }
    }

    function setIndexFromPreference() {
        if(ShotBrowserEngine.ready && !activeScopeIndex.valid && shotScopePref.value) {
            let i = getScopeIndex(shotScopePref.value)
            if(i && i.valid && activeScopeIndex != i)
                activeScopeIndex = i
        }
    }

    function getScopeIndex(scope_name) {
        let m = ShotBrowserEngine.presetsModel
        let p = m.searchRecursive("c5ce1db6-dac0-4481-a42b-202e637ac819", "idRole", ShotBrowserEngine.presetsModel.index(-1, -1), 0, 1)
        return m.searchRecursive(scope_name, "nameRole", p, 0, 1)
    }

    XsPreference {
        id: shotScopePref
        path: "/plugin/data_source/shotbrowser/shot_history/scope"

        onValueChanged: setIndexFromPreference()
    }

    onActiveScopeIndexChanged: {
        if(activeScopeIndex && activeScopeIndex.valid) {
            let m = activeScopeIndex.model
            let i = m.get(activeScopeIndex, "nameRole")
            shotScopePref.value = i
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

    Component.onCompleted: {
        if(visible) {
            ShotBrowserEngine.connected = true
            if(!ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid))
                runQuery()
        }
        setIndexFromPreference()
    }

    onVisibleChanged: {
        if(visible) {
            ShotBrowserEngine.connected = true
            setIndexFromPreference()
            if(!ShotBrowserHelpers.updateMetadata(isPanelEnabled, onScreenMediaUuid))
                runQuery()
        }
    }

    function runQuery() {
        if(isPanelEnabled && activeScopeIndex.valid) {
            // make sure the results appear in sync.
            queryCounter += 1
            let i = queryCounter
            Future.promise(
                ShotBrowserEngine.executeQuery(
                    [ShotBrowserEngine.presetsModel.get(activeScopeIndex, "jsonPathRole")])//, {"up": "down"})
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

    XsGradientRectangle{ id: backgroundDiv
        anchors.fill: parent
    }

    ShotHistoryResultPopup {
        id: resultPopup
        menu_model_name: "shot_history_popup"
        popupSelectionModel: resultsSelectionModel
    }

    ShotBrowserResultModel {
        id: results
    }

    ItemSelectionModel {
        id: resultsSelectionModel
        model: results
    }

    ColumnLayout{
        anchors.fill: parent
        spacing: 0

        ShotHistoryTitleDiv{id: titleDiv
            titleButtonHeight: (buttonHeight + 4)
            Layout.fillWidth: true
            Layout.preferredHeight: titleButtonHeight + (panelPadding*2)
        }

        ShotHistoryListDiv{ id: contentDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ShotHistoryActionDiv{id: buttonsDiv
            Layout.fillWidth: true
            Layout.preferredHeight: buttonHeight + (panelPadding*2)
        }
    }
}