// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.14
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import QuickFuture 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0

XsWindow{

    title: "Publish "+notesType+" Notes"
    property bool isPlaylistNotes: true
    property string notesType: isPlaylistNotes? "Playlist": "Selected Media"
    property string message: "No notes to publish."

    property real itemHeight: btnHeight
    property real itemSpacing: 1

    property int projectId: -1

    property alias notifyOwner: notifyCreatorCB.checked
    property alias combineNotes: combineNotesCB.checked
    property alias addFrameTimeCode: addFrameTimeCodeCB.checked
    property alias addPlaylistName: addPlaylistNameCB.checked
    property alias addNoteType: addNoteTypeCB.checked
    property alias ignoreWithOnlyDrawing: ignoreWithOnlyDrawingCB.checked
    property alias skipAlreadyPublished: skipAlreadyPublishedCB.checked
    property string defaultType: typeRenameDiv.checked ? prefs.values.defaultType : ""
    property var playlistUuid: null
    property var mediaUuids: []

    property var payload: null
    property var payload_obj: null

    property int notesCount: (payload_obj ? payload_obj["payload"].length : 0)


    onNotesCountChanged:{
        if(notesCount===1) message = "Ready to publish " +notesCount+" note."
        else if(notesCount>1) message = "Ready to publish " +notesCount+" notes."
        else message = "No notes to publish."
    }

    onPayloadChanged: {
        payload_obj = JSON.parse(payload)

        // find project id..
        if(payload_obj && payload_obj["payload"].length) {
            let id = payload_obj["payload"][0]["payload"]["project"]["id"]
            projectDiv.currentIndex = projectDiv.model.search(id, "idRole").row
        }
    }

    onPlaylistUuidChanged: {
        // update playlist combo, if required.
        for(let i = 0; i< theSessionData.playlists.length; i++) {
            let ustr = helpers.QUuidToQString(playlistUuid)
            if(theSessionData.playlists[i].uuid == ustr) {
                if(playlistDiv.currentIndex != i)
                    playlistDiv.currentIndex = i
                break
            }
        }

        updatePublish()
    }


    width: 400
    height: 620
    minimumWidth: 400
    // maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    // palette.base: XsStyleSheet.panelTitleBarColor

    XsSBPublishNotesFeedback {
        id: publish_notes_feedback
        property real btnHeight: XsStyleSheet.widgetStdHeight + 4
    }


    function updatePublish() {
        if(visible) {
            // console.log("playlistUuid", playlistUuid)
            // console.log("notifyOwner", notifyOwner)
            // console.log("combineNotes", combineNotes)
            // console.log("addFrameTimeCode", addFrameTimeCode)
            // console.log("addPlaylistName", addPlaylistName)
            // console.log("addNoteType", addNoteType)
            // console.log("ignoreWithOnlyDrawing", ignoreWithOnlyDrawing)
            // console.log("skipAlreadyPublished", skipAlreadyPublished)
            // console.log("defaultType", defaultType)

            if(playlistUuid) {
                payload = ShotBrowserEngine.preparePlaylistNotes(
                    playlistUuid,
                    mediaUuids,
                    notifyOwner,
                    getNotifyGroups(),
                    combineNotes,
                    addFrameTimeCode,
                    addPlaylistName,
                    addNoteType,
                    ignoreWithOnlyDrawing,
                    skipAlreadyPublished,
                    defaultType
                )
            }
        }
    }

    function publishNotes() {

        if(playlistUuid) {
            payload = ShotBrowserEngine.preparePlaylistNotes(
                playlistUuid,
                mediaUuids,
                notifyOwner,
                getNotifyGroups(),
                combineNotes,
                addFrameTimeCode,
                addPlaylistName,
                addNoteType,
                ignoreWithOnlyDrawing,
                skipAlreadyPublished,
                defaultType
            )

            Future.promise(
                ShotBrowserEngine.pushPlaylistNotesFuture(payload, playlistUuid)
            ).then(function(json_string) {
                console.log(json_string)

                publish_notes_feedback.isPlaylistNotes = isPlaylistNotes
                publish_notes_feedback.parseFeedback(json_string)
                publish_notes_feedback.show()
            })
        }
    }


    function getNotifyGroups() {
        let result = []
        let email_group_names = []
        if(notifyGroupCB.checked) {
            for(let i =0;i<notifyGroupCB.checkedIndexes.length;i++) {
                result.push(notifyGroupCB.model.sourceModel.get(notifyGroupCB.checkedIndexes[i], "idRole"))
                email_group_names.push(notifyGroupCB.model.sourceModel.get(notifyGroupCB.checkedIndexes[i], "nameRole"))
            }
        }

        return result
    }

    function publishFromPlaylist(playlist_uuid) {
        isPlaylistNotes = true
        mediaUuids = []
        playlistUuid = playlist_uuid
    }

    function publishFromMedia(medialist) {
        isPlaylistNotes = false
        mediaUuids = []

        if(medialist.length) {
            let m = medialist[0].model
            let plind = m.getPlaylistIndex(medialist[0])

            for(let i = 0; i< medialist.length; i++) {
                mediaUuids.push(m.get(medialist[i], "actorUuidRole"))
            }
            playlistUuid = m.get(plind, "actorUuidRole")
        }
    }


    XsModelNestedPropertyMap {
        id: prefs
        index: globalStoreModel.searchRecursive("/plugin/data_source/shotbrowser/note_publishing/note_publish_settings", "pathRole")
        property alias properties: prefs.values
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: itemSpacing

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight/2
        }
        XsTextWithComboBoxFullSize{ id: projectDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight*1.5

            enabled: false

            text: "Select project :"
            model: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Project") : []
            valueDiv.textRole: "nameRole"
            onCurrentIndexChanged: {
                if(currentIndex != -1) {
                    let pid = model.get(model.index(currentIndex,0), "idRole")
                    ShotBrowserEngine.cacheProject(pid)
                    projectId = pid
                }
            }
        }
        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight/4
        }
        XsTextWithComboBoxFullSize{ id: playlistDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight*1.5

            enabled: isPlaylistNotes

            text: "Select XSTUDIO playlist :"
            valueDiv.textRole: "text"
            model: theSessionData.playlists
            onCurrentIndexChanged: {
                if(currentIndex != -1) {
                    if(model[currentIndex].uuid != playlistUuid)
                        playlistUuid = model[currentIndex].uuid
                }
                else
                    playlistUuid = ""
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight
        }

        XsTextWithCheckAndComboBoxes { id: typeRenameDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            valueDiv.textRole: "nameRole"
            text: "Rename all note types :"

            checked: false
            currentIndex: -1
            model: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Note Type") : []

            onCurrentIndexChanged: {
                if(!checked && currentIndex != -1)
                    currentIndex = -1
            }

            onActivated: {
                if(index != -1) {
                    let dt = model.get(model.index(index, 0), "nameRole")
                    if(dt != prefs.values.defaultType)
                        prefs.values.defaultType = dt
                }
            }

            onCheckedChanged: {
                if(checked) {
                    if(prefs.values.defaultType == "")
                        currentIndex = -1
                    else
                        currentIndex = model.search(prefs.values.defaultType).row
                }
                else {
                    currentIndex = -1
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight
        }
        XsTextWithCheckBox { id: notifyCreatorCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Notify version creator"
            checked: prefs.values.notifyCreator
            onCheckedChanged: {
                if(prefs.values.notifyCreator != checked) {
                    updatePublish()
                    prefs.values.notifyCreator = checked
                }
            }
        }
        XsTextWithComboBoxMultiSelectable{ id: notifyGroupCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            z: 100
            text: "Notify :"
            hintText: "Recipients"
            checked: false
            model: ShotBrowserFilterModel {
                sourceModel: ShotBrowserEngine.ready ? ShotBrowserEngine.presetsModel.termModel("Group", "", projectId) : null
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight
        }

        XsTextWithCheckBox {
            id: combineNotesCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Combine multiple notes"
            checked: prefs.values.combine

            onCheckedChanged: {
                if(prefs.values.combine != checked) {
                    updatePublish()
                    prefs.values.combine = checked
                }
            }
        }

        XsTextWithCheckBox {
            id: addFrameTimeCodeCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Add 'Frame/Timecode Number' to notes"
            checked: prefs.values.addFrame
            onCheckedChanged: {
                if(prefs.values.addFrame != checked) {
                    updatePublish()
                    prefs.values.addFrame = checked
                }
            }
        }
        XsTextWithCheckBox {
            id: addPlaylistNameCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Add 'Playlist Name' to notes"
            checked: prefs.values.addPlaylistName
            onCheckedChanged: {
                if(prefs.values.addPlaylistName != checked) {
                    updatePublish()
                    prefs.values.addPlaylistName = checked
                }
            }
        }
        XsTextWithCheckBox {
            id: addNoteTypeCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Add 'Note Type' to notes"
            checked: prefs.values.addType
            onCheckedChanged: {
                if(prefs.values.addType != checked) {
                    updatePublish()
                    prefs.values.addType = checked
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight
        }
        XsTextWithCheckBox {
            id: ignoreWithOnlyDrawingCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Ignore notes with only drawings"
            checked: prefs.values.ignoreEmpty
            onCheckedChanged: {
                if(prefs.values.ignoreEmpty != checked) {
                    updatePublish()
                    prefs.values.ignoreEmpty = checked
                }
            }
        }
        XsTextWithCheckBox {
            id: skipAlreadyPublishedCB
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            text: "Skip notes already published"
            checked: prefs.values.skipAlreadyPublished
            onCheckedChanged: {
                if(prefs.values.skipAlreadyPublished != checked) {
                    updatePublish()
                    prefs.values.skipAlreadyPublished = checked
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Item{ id: msgDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            XsText{
                width: parent.width - itemSpacing*2
                height: message? itemHeight : 0
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                color: XsStyleSheet.accentColor //errorColor
                wrapMode: Text.Wrap
                text: message
            }
        }
        Item{ id: buttonsDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            RowLayout{
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 10


                XsPrimaryButton{
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width/2
                    Layout.fillHeight: true
                    text: "Cancel"
                    onClicked: {
                        forceActiveFocus()
                        close()
                    }
                }
                XsPrimaryButton{
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width/2
                    Layout.fillHeight: true
                    text: "Publish Notes To SG"
                    onClicked: {
                        forceActiveFocus()
                        publishNotes()
                        close()
                    }
                }

            }
        }
        Item{ id: infoDiv
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight

            XsText{
                x: 20
                width: parent.width - x*2
                height: itemHeight
                color: XsStyleSheet.secondaryTextColor
                font.pixelSize: XsStyleSheet.fontSize *0.9
                wrapMode: Text.Wrap
                text: "(Only notes attached to ShotGrid Media are currently supported.)"
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight/3
        }

    }



}

