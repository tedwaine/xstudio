import QtQuick 2.12
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import xstudio.qml.viewport 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

import "./widgets"
import "./hud"

// This Item extends the pure 'Viewport' QQuickItem from the cpp side

Viewport {

    id: view

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

    XsViewportOverlays {}

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
            repositionPopupMenu(
                menu_loader.item,
                view,
                mouse.x,
                mouse.y);
        }
    }

    // Cursor switching for pan/zoom modes

    XsModuleData {
        id: viewport_attrs
        modelDataName: view.name + "_attrs"
    }

    XsAttributeValue {
        id: is_zooming
        attributeTitle: "Zoom"
        model: viewport_attrs
    }

    XsAttributeValue {
        id: is_panning
        attributeTitle: "Pan"
        model: viewport_attrs
    }

    XsAttributeValue {
        id: __frame_rate_expr
        attributeTitle: "Frame Rate"
        model: viewport_attrs
    }
    property alias frame_rate_expr: __frame_rate_expr.value

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
            snapshotDialog = Qt.createQmlObject('import xStudioReskin 1.0; XsSnapshotDialog { viewerWidth:view.width; viewerHeight: view.height}', appWindow)
        }
        snapshotDialog.visible = true
    }

}
