import QtQuick 2.12
import QtQuick.Layouts 1.15
// import QtQml.Models 2.14

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import "."

Item{id: actionDiv
    width: parent.width;

    height: (btnHeight+(panelPadding*2))*opacity
    visible: opacity != 0.0
    Behavior on opacity {NumberAnimation{ duration: 150 }}

    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding

    property string actionbar_model_data_name

    /*************************************************************************

        Access Playhead data

    **************************************************************************/

    property var mediaIndex
    property var mediaUuid: viewportPlayhead.mediaUuid
    property var mediaSourceUuid: viewportPlayhead.mediaSourceUuid

    // When the current onscreen media changes, search for the corresponding
    // node in the main session data model
    onMediaUuidChanged: {

        // TODO - current this gets us to media actor, not media source actor,
        // so we can't get to the file name yet
        mediaIndex = theSessionData.searchRecursive(
            viewportPlayhead.mediaUuid,
            "actorUuidRole",
            viewedMediaSetIndex,
            0,
            2 // only go 2 levels deeper than the playlist
            )

    }

    onMediaSourceUuidChanged: lookupMediaSource()
    onMediaIndexChanged: lookupMediaSource()

    function lookupMediaSource() {
        if (mediaIndex.valid) {
            mediaSourceData.index = theSessionData.searchRecursive(
                viewportPlayhead.mediaSourceUuid,
                "actorUuidRole",
                mediaIndex
                )
        } else {
            mediaSourceData.index = theSessionData.index(-1, -1)
        }
    }

    // this gives us access to the 'role' data of the entry in the session model
    // for the current on-screen media
    XsModelPropertyMap {
        id: mediaData
        index: theSessionData.invalidIndex()
    }

     // this gives us access to the 'role' data of the entry in the session model
    // for the current on-screen media SOURCE
    XsModelPropertyMap {
        id: mediaSourceData
        property var fileName: {
            let result = "TBD"
            if(index.valid && values.pathRole != undefined) {
                result = helpers.fileFromURL(values.pathRole)
            }
            return result
        }
    }

    /*************************************************************************/

    RowLayout{
        x: panelPadding
        spacing: 1
        width: parent.width-(x*2)
        height: btnHeight
        anchors.verticalCenter: parent.verticalCenter

        XsPrimaryButton{ 
            visible: !isPopoutViewer
            id: popoutButton
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/open_in_new.svg"
            isActive: appWindow.popoutIsOpen
            onClicked: {
                appWindow.toggle_popout_viewer()
            }
        }

        XsModuleData {

            id: dockables
            modelDataName: "dockable viewport toolboxes"
        }
    
        Repeater {
            model: dockables
            XsPrimaryButton{ 
                Layout.preferredWidth: 40
                Layout.preferredHeight: parent.height
                imgSrc: icon_path
                isActive: visible_doc_widgets.indexOf(title) != -1
                //enabled: button_enabled
                onClicked: {
                    var isHidden = isActive
                    toggle_dockable_widget(title)
                    // set the 'activated' role data. This will send a message
                    // to the backend Module that the dockable widget has been
                    // either hidden or shown
                    activated = isHidden ? 0 : 1
                }
                onVisibleChanged: {
                    // if the button is hidden, this means the viewport has
                    // been hidden (e.g. by user changing a tab or layout)
                    // so we force the annotation tool off
                    if (!visible) {
                        activated = 0;
                    }
                }

                // in the backend, if a hotkey was provided when the dockable 
                // widget was declared, the hotkey press is signalled to us
                // by setting the user_data role data on the attribute - it
                // is a json including a 'context' key which tells us the name
                // of the viewport that the hotkey was pressed in. Here we
                // cross check with the name of the viewport
                property var userData: user_data
                onUserDataChanged: {
                    if (typeof userData == "object" && typeof userData.context == "string") {
                        if (userData.context == view.name) {
                            var isHidden = isActive
                            toggle_dockable_widget(title)
                            activated = isHidden ? 0 : 1
                        }
                    }
                }

            }
        }

        // additional shelf buttons to launch pop-out floating windows for
        // e.g. grading tools, notes panel
        Repeater {
            model: popoutWindowsModel
            XsPrimaryButton { 

                Layout.preferredWidth: 40
                Layout.preferredHeight: parent.height
                imgSrc: icon_path
                isActive: window_is_visible
                onClicked: {
                    window_is_visible = !window_is_visible
                }                
            }
        }

        XsText {

            Layout.fillWidth: true
            Layout.preferredHeight: parent.height
            text: mediaSourceData.fileName
            font.bold: true
            elide: Text.ElideMiddle

            MouseArea { id: toolTipMArea
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true

                onClicked:{
                    parent.elide = parent.elide == Text.ElideRight? Text.ElideMiddle : Text.ElideRight
                }

                XsToolTip{
                    text: parent.parent.text
                    visible: parent.containsMouse && parent.parent.truncated
                    width: parent.parent.textWidth //== 0? 0 : 150
                }
            }
        }

        XsPrimaryButton {
            id: resetBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/reset_tv.svg"
            onClicked:{
                view.reset()
            }
        }

        XsModuleData {
            id: actionbar_model_data
            modelDataName: actionbar_model_data_name
        }

        Repeater {

            id: the_view
            model: actionbar_model_data

            delegate: XsPrimaryButton{
                id: zoomBtn
                Layout.preferredWidth: 40
                Layout.preferredHeight: parent.height
                imgSrc: title == "Zoom" ? "qrc:/icons/zoom_in.svg" : "qrc:/icons/pan.svg"
                isActive: value
                onClicked:{
                    value = !value
                }
            }
        }

        /*XsPrimaryButton{ id: zoomBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/zoom_in.svg"
            isActive: isZoomMode
            property bool isZoomMode: false
            onClicked:{
                isZoomMode = !isZoomMode
                panBtn.isActive = false
            }
        }
        XsPrimaryButton{ id: panBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/pan.svg"
            onClicked:{
                isActive = !isActive
                zoomBtn.isZoomMode = false
            }
        }*/

        XsPrimaryButton{ id: moreBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/more_vert.svg"
            enabled: false
        }
    }

}