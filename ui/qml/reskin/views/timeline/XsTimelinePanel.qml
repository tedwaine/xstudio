// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15
import QtQml 2.14
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0

import xStudioReskin 1.0

Item{

    id: panel
    anchors.fill: parent

    property color bgColorPressed: palette.highlight
    property color bgColorNormal: "transparent"
    property color forcedBgColorNormal: bgColorNormal
    property color borderColorHovered: bgColorPressed
    property color borderColorNormal: "transparent"
    property real borderWidth: 1

    property real textSize: XsStyleSheet.fontSize
    property var textFont: XsStyleSheet.fontFamily
    property color textColorNormal: palette.text
    property color hintColor: XsStyleSheet.hintColor

    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding

    property string currentClipDetail: ""
    property var currentClipIndex: null

    property bool isEditToolsExpanded: false

    //#TODO: test
    property bool showIcons: false

    property alias theTimeline: theTimeline

    XsGradientRectangle{ id: bgDiiv
        z: -10
        anchors.fill: parent
    }

    // XsHotkey {
    //     id: fit_selection
    //     sequence: "F"
    //     name: "Fit Selection"
    //     description: "Fit Selection"
    //     onActivated: {
    //         console.log("FIT")
    //     }
    // }

    XsModelPropertyMap {
        id: currentClipProperties
        index: currentClipIndex || theSessionData.index(-1,-1)
        onContentChanged: updateCurrentClipDetail()
        onIndexChanged: updateCurrentClipDetail()
    }

    XsNewTrackMenu {
        id: newTrackMenu
    }

    function nTimer() {
         return Qt.createQmlObject("import QtQuick 2.0; Timer {}", root);
    }

    function delay(delayTime, cb) {
         let timer = new nTimer();
         timer.interval = delayTime;
         timer.repeat = false;
         timer.triggered.connect(cb);
         timer.start();
    }

    function updateCurrentClipDetail() {
        if(currentClipProperties.index && currentClipProperties.index.valid) {
            let model = currentClipProperties.index.model
            let tindex = model.getPlaylistIndex(currentClipProperties.index)
            let mlist = model.index(0, 0, tindex)
            let mediaIndex = model.search(currentClipProperties.values.clipMediaUuidRole, "actorUuidRole", mlist)
            if(model.canFetchMore(mediaIndex)) {
                model.fetchMore(mediaIndex)
                delay(250, function() {updateCurrentClipDetail()})
            } else {
                let mediaSourceIndex = model.search(
                    model.get(mediaIndex, "imageActorUuidRole"),
                    "actorUuidRole", mediaIndex
                )
                let taf = model.get(mediaSourceIndex, "timecodeAsFramesRole")
                if(taf == undefined) {
                    delay(250, function() {updateCurrentClipDetail()} )
                } else {
                    let name = currentClipProperties.values.nameRole
                    let start = currentClipProperties.values.trimmedStartRole
                    let astart = currentClipProperties.values.availableStartRole
                    let duration = currentClipProperties.values.trimmedDurationRole
                    let head = start - astart
                    let tail = currentClipProperties.values.availableDurationRole - head - duration

                    start = start - astart + taf
                    let end = start + duration - 1

                    currentClipDetail = "( "+head + " )   " + start + "   " + name + "   " + end + "   ( " + tail + " )"
                }
            }
        }
    }

    XsPlayhead {
        id: timelinePlayhead
    }

    property alias timelinePlayhead: timelinePlayhead

    Connections {

        target: theTimeline.timelineModel

        function onRootIndexChanged() {
            // connect to the timeline playhead ...
            let playhead_idx = theSessionData.searchRecursive(
                "Playhead",
                "typeRole",
                theTimeline.timelineModel.rootIndex.parent
                )

            if (playhead_idx.valid) {
                let playhead_uuid = theSessionData.get(playhead_idx, "actorUuidRole")
                if (playhead_uuid == undefined) {
                    // uh-oh - remember the session model is populated asynchronously
                    // we might need to wait few milliseconds until "actorUuidRole" for
                    // the playhead has been filled in.
                    callbackTimer.setTimeout(function(index) { return function() {
                        timelinePlayhead.uuid = theSessionData.get(playhead_idx, "actorUuidRole")
                        }}( playhead_idx ), 200);
                } else {
                    timelinePlayhead.uuid = playhead_uuid
                }
            }
        }
    }

    Connections {
        target: timelinePlayhead
        function onMediaUuidChanged() {
            updateClipIndex()
        }
    }

    function updateClipIndex() {
        let si = theTimeline.timelineSelection.selectedIndexes
        if(si.length == 1) {
            let i = si[0]
            if(i != currentClipIndex && i.model.get(i, "typeRole") == "Clip")
                currentClipIndex = i
        } else {
            currentClipIndex = theSessionData.getTimelineClipIndex(theTimeline.timelineModel.rootIndex, timelinePlayhead.logicalFrame)
        }
    }

    Connections {
        target: theTimeline.timelineSelection
        function onSelectionChanged(selected, deselected) {
            updateClipIndex()
        }
    }

    XsHotkeyArea {
        id: hotkey_area
        anchors.fill: parent
        context: "timeline"
        focus: true
    }

    Item{

        id: actionDiv
        width: parent.width;
        height: btnHeight+(panelPadding*2)

        RowLayout{
            x: panelPadding
            spacing: 1
            width: parent.width-(x*2)
            height: btnHeight
            anchors.verticalCenter: parent.verticalCenter

            XsPrimaryButton{ id: addPlaylistBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/add.svg"
                text: "Add"

                onClicked: {
                    // let m = theTimeline.timelineModel.srcModel
                    // let stack_index = m.index(0, 0, theTimeline.timelineModel.rootIndex)

                    // if(theTimeline.timelineSelection.selectedIndexes.length) {
                    //     let index = m.getTimelineTrackIndex(theTimeline.timelineSelection.selectedIndexes[0])
                    //     let type = m.get(index,"typeRole")
                    //     if(type == "Video Track")
                    //         theTimeline.addTrack("Video Track")
                    //     else if(type == "Audio Track")
                    //         theTimeline.addTrack("Audio Track")
                    // } else {
                        var pos = mapToItem(panel, x+width/2, y+height/2)
                        newTrackMenu.x = pos.x
                        newTrackMenu.y = pos.y
                        newTrackMenu.visible = true
                    // }
                }
            }
            XsPrimaryButton{ id: deleteBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/delete.svg"
                text: "Delete"
                onClicked: theTimeline.deleteItems(theTimeline.timelineSelection.selectedIndexes)
            }
            XsPrimaryButton{
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/undo.svg"
                text: "Undo"
                onClicked: theTimeline.undo(viewedMediaSetProperties.index)
            }

            XsPrimaryButton{
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/redo.svg"
                text: "Redo"
                onClicked:  theTimeline.redo(viewedMediaSetProperties.index)
            }

           XsPrimaryButton{
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/crop_free.svg"
                text: "Fit All"
                onClicked:  theTimeline.fitItems()
            }

           XsPrimaryButton{
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/fit_screen.svg"
                text: "Fit Selection"
                onClicked:  theTimeline.fitItems(theTimeline.timelineSelection.selectedIndexes)
            }

           XsPrimaryButton{
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/stacks.svg"
                text: "Flatten Selected Tracks"
                onClicked:  {
                    theSessionData.bakeTimelineItems(theTimeline.timelineSelection.selectedIndexes, "Flatten Track")
                    theTimeline.deleteItems(theTimeline.timelineSelection.selectedIndexes)
                }
            }

            XsSearchButton{ id: searchBtn
                Layout.preferredWidth: isExpanded? btnWidth*6 : btnWidth
                Layout.preferredHeight: parent.height
                isExpanded: false
                hint: "Search..."
                enabled: false
               // isExpandedToLeft: true
            }
            XsText{ id: titleDiv
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredHeight: parent.height
                text: currentClipDetail
                font.bold: true
                elide: Text.ElideMiddle
                visible: opacity != 0
                opacity: searchBtn.isExpanded ? 0 : 1
                Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.OutQuart }  }
            }
            XsPrimaryButton{
                Layout.preferredWidth: btnWidth*2.6
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/laps.svg"
                text: "Loop Selection"
                onClicked: theTimeline.loopSelection = !theTimeline.loopSelection
                isActive: theTimeline.loopSelection
            }
            XsPrimaryButton{
                Layout.preferredWidth: btnWidth*2.6
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/center_focus_weak.svg"
                text: "Focus Selection"
                onClicked: theTimeline.focusSelection = !theTimeline.focusSelection
                isActive: theTimeline.focusSelection
            }
            Item{
                Layout.preferredWidth: panelPadding/2
                Layout.preferredHeight: parent.height
            }

            // XsPrimaryButton{
            //     Layout.preferredWidth: showIcons? btnWidth : btnWidth*1.8
            //     Layout.preferredHeight: parent.height
            //     imgSrc: showIcons? "qrc:/icons/center_focus_strong.svg":""
            //     text: "Lock"
            //     isActive: theTimeline.selectionIsLocked
            //     onClicked:{
            //         if(theTimeline.selectionIsLocked)
            //             theTimeline.lockItems(theTimeline.timelineSelection.selectedIndexes, false)
            //         else
            //             theTimeline.lockItems(theTimeline.timelineSelection.selectedIndexes, true)
            //         theTimeline.updateLockFlag()
            //     }
            // }

            // XsPrimaryButton{
            //     Layout.preferredWidth: showIcons? btnWidth : btnWidth*1.8
            //     Layout.preferredHeight: parent.height
            //     imgSrc: showIcons? "qrc:/icons/center_focus_strong.svg":""
            //     text: "Enable"
            //     isActive: theTimeline.selectionIsEnabled
            //     onClicked:{
            //         if(theTimeline.selectionIsEnabled)
            //             theTimeline.enableItems(theTimeline.timelineSelection.selectedIndexes, false)
            //         else
            //             theTimeline.enableItems(theTimeline.timelineSelection.selectedIndexes, true)
            //         theTimeline.updateEnableFlag()
            //     }
            // }

            // XsPrimaryButton{
            //     Layout.preferredWidth: btnWidth*1.8
            //     Layout.preferredHeight: parent.height
            //     imgSrc: ""
            //     text: "Ripple"
            //     onClicked:{
            //         isActive = !isActive
            //     }
            //     enabled: false
            // }
            // XsPrimaryButton{
            //     Layout.preferredWidth: btnWidth*1.8
            //     Layout.preferredHeight: parent.height
            //     imgSrc: ""
            //     text: "Gang"
            //     onClicked:{
            //         isActive = !isActive
            //     }
            //     enabled: false
            // }
            // XsPrimaryButton{
            //     Layout.preferredWidth: btnWidth*1.8
            //     Layout.preferredHeight: parent.height
            //     imgSrc: ""
            //     text: "Snap"
            //     onClicked:{
            //         isActive = !isActive
            //     }
            //     enabled: false
            // }
            // Item{
            //     Layout.preferredWidth: panelPadding/2
            //     Layout.preferredHeight: parent.height
            // }
            // XsPrimaryButton{
            //     Layout.preferredWidth: btnWidth*1.8
            //     Layout.preferredHeight: parent.height
            //     imgSrc: ""
            //     text: "Overwrite"
            //     enabled: false
            // }
            // XsPrimaryButton{
            //     Layout.preferredWidth: btnWidth*1.8
            //     Layout.preferredHeight: parent.height
            //     imgSrc: ""
            //     text: "Insert"
            //     enabled: false
            // }
            // Item{
            //     Layout.preferredWidth: panelPadding/2
            //     Layout.preferredHeight: parent.height
            // }
            // XsPrimaryButton{ id: settingsBtn
            //     Layout.preferredWidth: btnWidth
            //     Layout.preferredHeight: parent.height
            //     imgSrc: "qrc:/icons/settings.svg"
            //     enabled: false
            // }
            // XsPrimaryButton{ id: filterBtn
            //     Layout.preferredWidth: btnWidth
            //     Layout.preferredHeight: parent.height
            //     imgSrc: "qrc:/icons/filter.svg"
            //     enabled: false
            // }
            // XsPrimaryButton{ id: morePlaylistBtn
            //     Layout.preferredWidth: btnWidth
            //     Layout.preferredHeight: parent.height
            //     imgSrc: "qrc:/icons/more_vert.svg"
            //     enabled: false
            // }

        }

    }

    Rectangle{

        id: timelineDiv
        x: panelPadding
        y: actionDiv.height
        width: panel.width-(x*2)
        height: panel.height-y-panelPadding
        color: XsStyleSheet.panelBgColor

        XsTimelineEditTools{

            x: spacing
            y: spacing

            width: isEditToolsExpanded? cellWidth*2 : cellWidth
            height: parent.height<cellHeight*(model.count/2)+cellHeight? cellHeight*(model.count/2)+cellHeight : parent.height

            onHeightChanged:{
                if(height<cellWidth*toolsModel.count) isEditToolsExpanded = true
                else isEditToolsExpanded = false
            }

            toolsModel: editToolsModel

            onCurrentIndexChanged: theTimeline.editMode = toolsModel.get(currentIndex)._name
        }

    }

    XsTimeline
    {
        id: theTimeline
        // TODO: do this with proper anchors. Can't seem to do it
        // without going over the edit buttons ....
        y: actionDiv.height
        width: parent.width - 40
        height: parent.height - actionDiv.height
        x: 40

    }

    ListModel{ id: editToolsModel

        ListElement{ _enabled: true; _type:"basic"; _name:"Select"; _icon:"qrc:/icons/arrow_selector_tool.svg"}
        // ListElement{ _enabled: true; _type:"basic"; _name:"Select Area"; _icon:"qrc:/icons/select.svg"}
        // ListElement{ _enabled: true; _type:"basic"; _name:"Focus"; _icon:"qrc:/icons/center_focus_strong.svg"}
        ListElement{ _enabled: true; _type:"basic"; _name:"Move"; _icon:"qrc:/icons/open_with.svg"}
        // ListElement{ _type:"basic"; _name:"Move LR"; _icon:"qrc:/icons/arrows_outward.svg"}
        // ListElement{ _type:"basic"; _name:"Move UD"; _icon:"qrc:/icons/arrows_outward.svg"}
        ListElement{ _enabled: false; _type:"basic"; _name:"Cut"; _icon:"qrc:/icons/content_cut.svg"}
        ListElement{ _enabled: true; _type:"basic"; _name:"Roll"; _icon:"qrc:/icons/expand.svg"}
        ListElement{ _enabled: false; _type:"basic"; _name:"Reorder"; _icon:"qrc:/icons/repartition.svg"}

    }

}
