import QtQuick 2.12
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import xstudio.qml.viewport 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

import "./widgets"

//Viewport {
Rectangle{ 
    
    color: "transparent"
    id: viewportWidget
    anchors.fill: parent
    property color gradient_colour_top: "#5C5C5C"
    property color gradient_colour_bottom: "#474747"

    property alias view: view
    focus: true

    Item {
        anchors.fill: parent
        // Keys.forwardTo: viewportWidget //#TODO: To check with Ted
        focus: true
        Keys.forwardTo: view
    }

    property real panelPadding: elementsVisible ? XsStyleSheet.panelPadding : 0

    Behavior on panelPadding {NumberAnimation{ duration: 150 }}

    Rectangle{
        id: r
        gradient: Gradient {
            GradientStop { position: r.alpha; color: gradient_colour_top }
            GradientStop { position: r.beta; color: gradient_colour_bottom }
        }
        anchors.fill: actionBar
        property real alpha: -y/height
        property real beta: parent.height/height + alpha

    }
    // XsGradientRectangle{ id: backgroundDiv //#TODO
    //     anchors.fill: actionBar
    //     property real alpha: -y/height
    //     property real beta: parent.height/height + alpha
    // }

    XsViewportActionBar{
        id: actionBar
        anchors.top: parent.top
        actionbar_model_data_name: view.name + "_actionbar"
        opacity: elementsVisible ? 1.0 : 0.0

    }


    Rectangle{
        id: r2
        gradient: Gradient {
            GradientStop { position: r2.alpha; color: gradient_colour_top }
            GradientStop { position: r2.beta; color: gradient_colour_bottom }
        }
        anchors.top: infoBar.top
        anchors.bottom: infoBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        property real alpha: -y/height
        property real beta: parent.height/height + alpha

    }

    XsViewportInfoBar{
        id: infoBar
        anchors.top: actionBar.bottom
        opacity: elementsVisible ? 1.0 : 0.0
    }
    
    property color gradient_dark: "black"
    property color gradient_light: "white"

    XsModuleData {

        id: viewportPlayheadDataModel

        // each playhead exposes its attributes in a model named after the
        // playhead UUID. We connect to the model like this:
        modelDataName: view.playhead.uuid
    }
    property alias viewportPlayheadDataModel: viewportPlayheadDataModel

    /*
    // Leaving this here for posterity. Uncommenting this will result in
    // debug spam telling you about data in viewportPlayheadDataModel.
    // It gives you an idea about what 'XsModuleData' data does.
    Repeater {
        model: viewportPlayheadDataModel
        Item {
            property var value_: value
            onValue_Changed: {
                console.log("playhead attr changed", title, value)
            }
            Component.onCompleted: {
                console.log("playhead attr: ", title, value)
            }
        }
    }
    */

    Viewport {

        id: view
        x: panelPadding
        y: (actionBar.height + infoBar.height)
        width: parent.width-(x*2)
        height: parent.height-(toolBar.height + transportBar.height) - (y)
        z: -100

        XsHotkeyArea {
            anchors.fill: parent
            context: view.name
            focus: true
        }

        /**************************************************************

        HOTKEYS

        ****************************************************************/
        XsHotkey {
            id: hide_ui_hotkey
            sequence: "Ctrl+H"
            name: "Hide UI"
            description: "Hides/reveals the toolbars and info bars in the viewer"
            context: view.name
            onActivated: {
                elementsVisible = !elementsVisible
            }
        }

        property alias hide_ui_hotkey: hide_ui_hotkey

        onPointerEntered: {
            //appWindow.setFocusViewport(view)
        }

        onVisibleChanged: {
            if (visible) {
                //appWindow.setFocusViewport(view)
            }
        }    

        // this one lays out the HUD graphics coming from HUD plugins and
        // also general overlay graphics like Mask
        XsViewportHUD {}

        // This is needed to connect a new viewportWidget to the current viewed 
        // playhead when viewports are created via the panels features
        Component.onCompleted: {
            // get index to the playhead of the current viewed media set (playlist, subset or timeline)
            let ind = theSessionData.searchRecursive("Playhead", "typeRole", viewedMediaSetIndex)

            // get the actor address
            let playheadAddr = theSessionData.get(ind, "actorRole")

            // set out playhead
            setPlayhead(playheadAddr)
        }

        Loader {
            id: menu_loader
        }
    
        // This menu is built from a menu model that is maintained by xSTUDIO's
        // backend. We access the menu model by an id string 'menuModelName' that
        // will be set by the derived type
        Component {
            id: menuComponent
            XsViewerContextMenu {
                viewport: view
            }
        }
                        
        onMouseButtonsChanged: {
            if (mouseButtons == Qt.RightButton) {
                if (menu_loader.item == undefined) {
                    menu_loader.sourceComponent = menuComponent
                }
                showPopupMenu(
                    menu_loader.item,
                    view,
                    mouse.x,
                    mouse.y);
            }
        }
    
        // Cursor switching for pan/zoom modes

        XsModuleData {
            id: zoom_pan_state_model
            modelDataName: view.name + "_pan_zoom"
        }
    
        XsAttributeValue {
            id: is_zooming
            attributeTitle: "Zoom"
            model: zoom_pan_state_model
        }
    
        XsAttributeValue {
            id: is_panning
            attributeTitle: "Pan"
            model: zoom_pan_state_model
        }

        property var is_normal_cursor: is_zooming.value ? false : is_panning.value ? false : true
        onIs_normal_cursorChanged: {
            if (is_normal_cursor)
                setOverrideCursor("", false);
            else if (is_zooming.value)
                setOverrideCursor("://cursors/magnifier_cursor.svg", true)
            else if (is_panning.value)
            setOverrideCursor(Qt.OpenHandCursor)
        }

        property var snapshotDialog
        function doSnapshot() {
            studio.setupSnapshotViewport(view.playheadActorAddress())
            if (snapshotDialog == undefined) {
                snapshotDialog = Qt.createQmlObject('import xStudioReskin 1.0; XsSnapshotDialog {}', appWindow)
            }
            snapshotDialog.visible = true
        }

    }

    Rectangle{
        // couple of pixels down the left of the viewportWidget
        id: left_side
        gradient: Gradient {
            GradientStop { position: left_side.alpha; color: gradient_colour_top }
            GradientStop { position: left_side.beta; color: gradient_colour_bottom }
        }
        anchors.left: parent.left
        anchors.right: view.left
        anchors.top: view.top
        anchors.bottom: view.bottom
        property real alpha: -y/height
        property real beta: parent.height/height + alpha

    }

    Rectangle{
        // couple of pixels down the right of the viewportWidget
        id: right_side
        gradient: Gradient {
            GradientStop { position: right_side.alpha; color: gradient_colour_top }
            GradientStop { position: right_side.beta; color: gradient_colour_bottom }
        }
        anchors.left: view.right
        anchors.right: parent.right
        anchors.top: view.top
        anchors.bottom: view.bottom
        property real alpha: -y/height
        property real beta: parent.height/height + alpha

    }

    Rectangle{
        id: r3
        gradient: Gradient {
            GradientStop { position: r3.alpha; color: gradient_colour_top }
            GradientStop { position: r3.beta; color: gradient_colour_bottom }
        }
        anchors.fill: toolBar
        property real alpha: -y/height
        property real beta: parent.height/height + alpha

    }

    XsViewportToolBar{
        id: toolBar
        anchors.bottom: transportBar.top
        toolbar_model_data_name: view.name + "_toolbar"
        opacity: elementsVisible ? 1.0 : 0.0
    }

    Rectangle{
        id: r4
        gradient: Gradient {
            GradientStop { position: r4.alpha; color: gradient_colour_top }
            GradientStop { position: r4.beta; color: gradient_colour_bottom }
        }
        anchors.fill: transportBar
        property real alpha: -y/height
        property real beta: parent.height/height + alpha

    }

    XsViewportTransportBar { 
        id: transportBar
        anchors.bottom: parent.bottom
        opacity: elementsVisible ? 1.0 : 0.0
    }

    property bool elementsVisible: true

}