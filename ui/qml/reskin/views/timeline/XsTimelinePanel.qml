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

XsGradientRectangle{

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

    property var currentClipRange: [-1,-1]
    property var currentClipHandles: [0,0]
    property var currentClipIndex: null

    //#TODO: test
    property bool showIcons: false

    property alias theTimeline: theTimeline

    enabled: theTimeline.have_timeline

    XsModelPropertyMap {
        id: currentClipProperties
        index: currentClipIndex || theSessionData.index(-1,-1)
        onContentChanged: updateCurrentClipDetail()
        onIndexChanged: updateCurrentClipDetail()
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

                    currentClipRange = [start, end]
                    currentClipHandles = [head, tail]
                    // currentClipDetail = "( "+head + " )   " + start + "   " + name + "   " + end + "   ( " + tail + " )"
                }
            }
        }
    }

    XsPlayhead {
        id: timelinePlayhead
        Component.onCompleted: {
            connectToModel()
        }
        function connectToModel() {

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

    property alias timelinePlayhead: timelinePlayhead

    Connections {

        target: theTimeline.timelineModel

        function onRootIndexChanged() {

            timelinePlayhead.connectToModel()
        }
    }

    Connections {
        target: timelinePlayhead
        function onMediaUuidChanged() {
            updateClipIndex()
        }
    }

    function updateClipIndex() {
        // let si = theTimeline.timelineSelection.selectedIndexes
        // if(si.length == 1) {
        //     let i = si[0]
        //     if(i != currentClipIndex && i.model.get(i, "typeRole") == "Clip")
        //         currentClipIndex = helpers.makePersistent(i)
        // } else {
            currentClipIndex = helpers.makePersistent(theSessionData.getTimelineClipIndex(theTimeline.timelineModel.rootIndex, timelinePlayhead.logicalFrame))
        // }
    }

    // Connections {
    //     target: theTimeline.timelineSelection
    //     function onSelectionChanged(selected, deselected) {
    //         updateClipIndex()
    //     }
    // }

    XsHotkeyArea {
        id: hotkey_area
        anchors.fill: parent
        context: "timeline"
        focus: true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4

        RowLayout{
            spacing: 2
            Layout.maximumHeight: btnHeight
            Layout.minimumHeight: btnHeight
            Layout.fillWidth: true
            Layout.leftMargin: btnWidth + 4

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
                Layout.leftMargin: 24
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/crop_free.svg"
                text: "Fit All"
                onClicked:  theTimeline.fitItems()
            }

           XsPrimaryButton{
                Layout.rightMargin: 24
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
                    theSessionData.bakeTimelineItems(theTimeline.timelineSelection.selectedIndexes)
                    theTimeline.deleteItems(theTimeline.timelineSelection.selectedIndexes)
                }
            }

           XsPrimaryButton{
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/splitscreen_add.svg"
                text: "Insert Track Above"
                onClicked:  theTimeline.insertTrackAbove(theTimeline.timelineSelection.selectedIndexes)
            }

           XsPrimaryButton{
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                Layout.rightMargin: 24
                imgSrc: "qrc:/icons/library_add.svg"
                text: "Duplicate Selected"
                onClicked: theTimeline.duplicate(theTimeline.timelineSelection.selectedIndexes)
            }

            XsSearchButton{ id: searchBtn
                Layout.preferredWidth: isExpanded? btnWidth*6 : btnWidth
                Layout.preferredHeight: parent.height
                isExpanded: false
                hint: "Search..."
                enabled: false
               // isExpandedToLeft: true
            }

            Item{
                Layout.fillWidth: true
            }

            XsPrimaryButton{
                Layout.preferredWidth: btnWidth*4
                Layout.preferredHeight: parent.height
                showBoth: true
                imgSrc: "qrc:/icons/laps.svg"
                text: "Loop Selection"
                onClicked: theTimeline.loopSelection = !theTimeline.loopSelection
                isActive: theTimeline.loopSelection
            }
            XsPrimaryButton{
                Layout.preferredWidth: btnWidth*4
                Layout.preferredHeight: parent.height
                showBoth: true
                imgSrc: "qrc:/icons/center_focus_weak.svg"
                text: "Focus Selection"
                onClicked: theTimeline.focusSelection = !theTimeline.focusSelection
                isActive: theTimeline.focusSelection
            }

            Item{
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.preferredWidth: btnWidth*5
                Layout.maximumWidth: btnWidth*5
                Layout.fillHeight: true

                ColumnLayout {
                    Layout.preferredWidth: btnWidth*1.5
                    Layout.maximumWidth: btnWidth*1.5
                    Layout.fillHeight: true

                    XsText{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: "Range:"
                        horizontalAlignment: Text.AlignRight
                    }

                    XsText{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: "Handles:"
                        horizontalAlignment: Text.AlignRight
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    XsText{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: currentClipRange[0]
                        horizontalAlignment: Text.AlignRight
                    }

                    XsText{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: Math.abs(currentClipHandles[0]) + (currentClipHandles[0] < 0 ? " -" : "")
                        horizontalAlignment: Text.AlignRight
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    XsText{
                        Layout.fillHeight: true
                        font.bold: true
                        text: " - "
                    }

                    XsText{
                        Layout.fillHeight: true
                        font.bold: true
                        text: " / "
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    XsText{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: currentClipRange[1]
                        horizontalAlignment: Text.AlignLeft
                    }

                    XsText{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: Math.abs(currentClipHandles[1]) + (currentClipHandles[1] < 0 ? " -" : "")
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }

        Rectangle{
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: theTimeline.trackBackground

            RowLayout {
                anchors.fill: parent
                Rectangle{
                    Layout.fillHeight: true
                    Layout.minimumWidth: btnWidth
                    Layout.maximumWidth: btnWidth
                    color: theTimeline.trackBackground

                    ColumnLayout {
                        anchors.fill: parent

                        XsPrimaryButton{
                            Layout.minimumHeight: btnHeight
                            Layout.maximumHeight: btnHeight
                            Layout.fillWidth: true
                            text: "Select"
                            isActiveIndicatorAtLeft: true
                            imgSrc: "qrc:/icons/arrow_selector_tool.svg"
                            isActive: theTimeline.editMode == text
                            onClicked: theTimeline.editMode = text
                        }

                        XsPrimaryButton{
                            Layout.minimumHeight: btnHeight
                            Layout.maximumHeight: btnHeight
                            Layout.fillWidth: true
                            text: "Move"
                            isActiveIndicatorAtLeft: true
                            imgSrc: "qrc:/icons/open_with.svg"
                            isActive: theTimeline.editMode == text
                            onClicked: theTimeline.editMode = text
                        }

                        XsPrimaryButton{
                            Layout.minimumHeight: btnHeight
                            Layout.maximumHeight: btnHeight
                            Layout.fillWidth: true
                            text: "Roll"
                            isActiveIndicatorAtLeft: true
                            imgSrc: "qrc:/icons/expand.svg"
                            onClicked: theTimeline.editMode = text
                            isActive: theTimeline.editMode == text
                            imageDiv.rotation: 90
                        }

                        Item {
                            Layout.fillHeight: true
                        }

                        // XsPrimaryButton{
                        //     text: "Cut"
                        //     isActiveIndicatorAtLeft: true
                        //     imgSrc: "qrc:/icons/content_cut.svg"
                        //     onClicked: theTimeline.editMode = text
                        // }

                        // XsPrimaryButton{
                        //     text: "Reorder"
                        //     isActiveIndicatorAtLeft: true
                        //     imgSrc: "qrc:/icons/repartition.svg"
                        //     onClicked: theTimeline.editMode = text
                        // }


                    }
                }

                XsTimeline {
                    id: theTimeline
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
}
