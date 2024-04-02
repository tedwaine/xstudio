import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
// import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import "./widgets"

Item {
    id: transportBar
    width: parent.width

    height: (btnHeight+(barPadding*2))*opacity
    visible: opacity != 0.0
    Behavior on opacity {NumberAnimation{ duration: 150 }}

    property string panelIdForMenu: panelId

    property real barPadding: XsStyleSheet.panelPadding
    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight+(2*2)

    /*************************************************************************

        Access Playhead data

    **************************************************************************/
    XsAttributeValue {
        id: __playheadPlaying
        attributeTitle: "playing"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadForward
        attributeTitle: "forward"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadFFWD
        attributeTitle: "Velocity Multiplier"
        model: viewportPlayheadDataModel
    }
    XsAttributeValue {
        id: __playheadLoopMode
        attributeTitle: "Loop Mode"
        model: viewportPlayheadDataModel
    }

    property alias playheadPlaying: __playheadPlaying.value
    property alias fastForward: __playheadFFWD.value
    property alias playingForwards: __playheadForward.value
    property alias playheadLoopMode: __playheadLoopMode.value
    /*************************************************************************/

    function skipToNext(forwards) {

        // jumpToNext will return false if there is only one source selected
        if (forwards && view.playhead.jumpToNextSource()) {
            return;
        } else if (view.playhead.jumpToPreviousSource()) {
            return;
        }

        // move selection forwards or backwards
        if (mediaSelectionModel.selectedIndexes.length == 1) {
            var idx = mediaSelectionModel.selectedIndexes[0]
            var idx2 = idx.model.index(idx.row + (forwards ? 1 : -1),0,idx.parent)
            if (idx2.valid)
                mediaSelectionModel.select(idx2, ItemSelectionModel.ClearAndSelect)
        }

    }

    function fastPlayback(rewind) {
        if (!rewind && !playingForwards) {
            // playing backwards but fast forward was hit ... play forwards
            // at normal speed
            playingForwards = true
            fastForward = 1.0
        } else if (rewind && playingForwards) {
            // playing forwards but fast rewind was hit ... play backwards
            // at normal speed
            playingForwards = false
            fastForward = 1.0
        } else {
            if (fastForward == 16.0) {
                fastForward = 1.0
            } else {
                fastForward = fastForward*2.0;
            }
        }
    }

    RowLayout{
        x: barPadding
        spacing: barPadding
        width: parent.width-(x*2)
        height: btnHeight
        anchors.verticalCenter: parent.verticalCenter

        RowLayout{
            spacing: 1
            Layout.preferredWidth: btnWidth*5
            Layout.maximumHeight: parent.height

            XsPrimaryButton{ 
                id: rewindButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/fast_rewind.svg"
                onClicked: fastPlayback(true)
                isActive: !playingForwards && fastForward > 1
                XsLabel {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 3
                    color: palette.highlight
                    text: fastForward > 1 ? "x" + fastForward : ""
                    shadow: true
                    visible: rewindButton.isActive
                }
            }
            XsPrimaryButton{ id: previousButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/skip_previous.svg"
                onPressed: skipToNext(false)
            }
            XsPrimaryButton{ id: playButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: playheadPlaying ? "qrc:/icons/pause.svg" : "qrc:/icons/play_arrow.svg"
                onClicked: playheadPlaying = !playheadPlaying
            }
            XsPrimaryButton{ id: nextButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/skip_next.svg"
                onPressed: skipToNext(true)
            }
            XsPrimaryButton{ id: forwardButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/fast_forward.svg"
                onClicked: fastPlayback(false)
                isActive: (playingForwards == true && fastForward > 1.0)
                XsLabel {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 3
                    color: palette.highlight
                    text: fastForward > 1 ? "x" + fastForward : ""
                    shadow: true
                    visible: forwardButton.isActive
                }

            }
        }

        XsCurrentFrameIndicator {
            id: currentFrameInd
            fontSize: 14
        }

        Rectangle {
            color: "black"
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height
            XsViewerTimeline {
                anchors.fill: parent
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                anchors.topMargin: 2
                anchors.bottomMargin: 2

            }

        }
        

        XsDurationFPSIndicator {
            timelineUnits: currentFrameInd.timelineUnits
            fontSize: 14
        }

        RowLayout{
            spacing: 1
            Layout.preferredHeight: parent.height

            XsViewerVolumeButton{ 
                id: volumeButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                volume: 4
            }

            XsPrimaryButton{ 
                
                id: loopModeButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: playheadLoopMode == "Loop" ? "qrc:/icons/repeat.svg" : playheadLoopMode == "Play Once" ? "qrc:/icons/keyboard_tab.svg" : "qrc:/icons/arrow_range.svg"
                isActive: loopModeBtnMenu.visible
                enabled: playheadLoopMode != undefined

                onClicked: {
                    loopModeBtnMenu.x = x-width//*2
                    loopModeBtnMenu.y = y-loopModeBtnMenu.height
                    loopModeBtnMenu.visible = !loopModeBtnMenu.visible
                }

                XsMenuNew {
                    id: loopModeBtnMenu
                    // visible: false
                    menu_model: loopModeBtnMenuModel
                    menu_model_index: loopModeBtnMenuModel.index(-1, -1)
                    menuWidth: 100
                }

                XsMenusModel {
                    id: loopModeBtnMenuModel
                    modelDataName: "LoopModeMenu-"+panelIdForMenu
                    onJsonChanged: {
                        loopModeBtnMenu.menu_model_index = index(-1, -1)
                    }
                }

                XsMenuModelItem {
                    menuPath: ""
                    menuItemType: "radiogroup"
                    menuItemPosition: 1
                    choices: ["Play Once", "Loop", "Ping Pong"]
                    currentChoice: playheadLoopMode ? playheadLoopMode : "Loop"
                    property var lm: playheadLoopMode
                    onLmChanged: {
                        if (playheadLoopMode != currentChoice && playheadLoopMode != undefined) {
                            currentChoice = playheadLoopMode
                        }                        
                    }
                    onCurrentChoiceChanged: {
                        if (playheadLoopMode != currentChoice) {
                            playheadLoopMode = currentChoice
                        }
                    }
                    menuModelName: "LoopModeMenu-"+panelIdForMenu
                }
            
            }

            XsPrimaryButton{ 
                id: snapshotButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/photo_camera.svg"
                onClicked: {                    
                    view.doSnapshot()
                }
            }
            XsPrimaryButton{ 
                visible: !isPopoutViewer
                id: popoutButton
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/open_in_new.svg"
                isActive: appWindow.popoutIsOpen
                onClicked: {
                    appWindow.toggle_popout_viewer()
                }
            }

        }
    }

}