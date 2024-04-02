import QtQuick 2.12
import QtQuick.Layouts 1.15
// import QtQml.Models 2.14

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import "./widgets"

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

    // Get the UUID of the current onscreen media from the playhead
    XsModelProperty {
        id: __playheadSourceUuid
        role: "value"
        index: viewportPlayheadDataModel.searchRecursive("Current Media Uuid", "title")
    }
    XsModelProperty {
        id: __playheadMediaSourceUuid
        role: "value"
        index: viewportPlayheadDataModel.searchRecursive("Current Media Source Uuid", "title")
    }

    Connections {
        target: viewportPlayheadDataModel // this bubbles up from XsSessionWindow
        function onJsonChanged() {
            __playheadSourceUuid.index = viewportPlayheadDataModel.searchRecursive("Current Media Uuid", "title")
            __playheadMediaSourceUuid.index = viewportPlayheadDataModel.searchRecursive("Current Media Source Uuid", "title")
        }
    }
    property alias mediaUuid: __playheadSourceUuid.value
    property alias mediaSourceUuid: __playheadMediaSourceUuid.value

    // When the current onscreen media changes, search for the corresponding
    // node in the main session data model
    onMediaUuidChanged: {

        // TODO - current this gets us to media actor, not media source actor,
        // so we can't get to the file name yet
        mediaData.index = theSessionData.searchRecursive(
            mediaUuid,
            "actorUuidRole",
            viewedMediaSetIndex
            )
    }

    onMediaSourceUuidChanged: {
        mediaSourceData.index = theSessionData.searchRecursive(
            mediaSourceUuid,
            "actorUuidRole",
            viewedMediaSetIndex
            )
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

        XsPrimaryButton{ id: transformBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/open_with.svg"
            enabled: false
            onClicked:{
                isActive = !isActive
            }
        }
        XsPrimaryButton{ id: colourBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/tune.svg"
            enabled: false
            onClicked:{
                isActive = !isActive
            }
        }
        XsPrimaryButton{ id: drawBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/brush.svg"
            enabled: false
            onClicked:{
                isActive = !isActive
            }
        }
        XsPrimaryButton{ id: notesBtn
            Layout.preferredWidth: 40
            Layout.preferredHeight: parent.height
            imgSrc: "qrc:/icons/sticky_note.svg"
            enabled: false
            onClicked:{
                isActive = !isActive
            }
        }
        XsText{
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