// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0
import QtQml.Models 2.14
import QtQuick.Layouts 1.4

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.session 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.viewport 1.0

import "./quickview/"

ApplicationWindow {

    id: appWindow
    visible: true
    color: "#00000000"
    title: "xSTUDIO Re-Skin UI (Under Construction)"

    property var window_name: "main_window"

    // override default palette
    palette.base: XsStyleSheet.panelBgColor
    palette.highlight: XsStyleSheet.accentColor //== "#666666" ? Qt.lighter(XsStyleSheet.accentColor, 1.5) : XsStyleSheet.accentColor
    palette.text: XsStyleSheet.primaryTextColor
    palette.buttonText: XsStyleSheet.primaryTextColor
    palette.windowText: XsStyleSheet.primaryTextColor
    palette.button: Qt.darker("#414141", 2.4)
    palette.light: "#bb7700"
    palette.highlightedText: Qt.darker("#414141", 2.0)
    palette.brightText: "#bb7700"

    FontLoader {id: fontInter; source: "qrc:/fonts/Inter/Inter-Regular.ttf"}



    property bool isFlatTheme: false
    
    Timer { id: gradTimer //#TODO
        running: false
        repeat: true
        interval: 3000

        onTriggered: {
            isFlatTheme = !isFlatTheme
        }
    }


    XsReskinPanelsLayoutModel {

        id: ui_layouts_model
        property var root_index: ui_layouts_model.index(-1, -1)
        onJsonChanged: {
            var idx = ui_layouts_model.searchRecursive(window_name, "window_name")
            if (layoutBar.selected_layout == -1) {
                layoutBar.selected_layout = ui_layouts_model.get(idx, "current_layout")
            }
            root_index = idx
            appWindow.x = ui_layouts_model.get(ui_layouts_model.root_index, "position_x")
            appWindow.y = ui_layouts_model.get(ui_layouts_model.root_index, "position_y")
            appWindow.width = ui_layouts_model.get(ui_layouts_model.root_index, "width")
            appWindow.height = ui_layouts_model.get(ui_layouts_model.root_index, "height")
            numLayouts = ui_layouts_model.rowCount(root_index)
        }
        property var numLayouts: 0

    }

    XsPreference {
        id: __tabsVisibility
        index: globalStoreModel.searchRecursive("/ui/qml/tabs_visibility", "pathRole")
    }

    property alias tabsVisibility: __tabsVisibility.value
    property var layoutName: ui_layouts_model.get(layoutBar.current_layout_index, "layout_name")
    property var lastNonPresentLayout
    onLayoutNameChanged: {
        if (layoutName != "Present") {
            lastNonPresentLayout = layoutName
        }
    }
    function setLayoutName(name) {
        var idx = ui_layouts_model.searchRecursive(name, "layout_name", ui_layouts_model.root_index)
        if (idx.valid) {
            layoutBar.selected_layout = idx.row
        } else if (layoutBar.current_layout_index.row > 0) {
            layoutBar.selected_layout = layoutBar.current_layout_index.row-1
        } else if (layoutBar.current_layout_index.model.rowCound(layoutBar.current_layout_index.parent)) {
            layoutBar.selected_layout = layoutBar.current_layout_index.row+1
        }
    }

    function togglePresentationMode() {
        if (layoutName == "Present") {
            setLayoutName(lastNonPresentLayout)
        } else {
            setLayoutName("Present")
        }
    }

    XsFileFunctions {
        id: file_functions
    }
    property alias file_functions: file_functions

    // if we want to show a pop-up in a particular place, we need to work out
    // the pop-up coordinates relative to it's parent. This can be a bit awkward
    // if we are doing it relative to some button that is not the parent of
    // the pop-up. This convenience function makes that easy ... it also makes
    // sure menus don't get placed so they go outside the application window
    function showPopupMenu(menu, refItem, refX, refY, altRightEdge) {
        var global_pos = refItem.mapToItem(
            appWindow.contentItem,
            refX,
            refY
            )

        if (altRightEdge !== undefined) {

            // altRightEdge allows us to position a pop-up to the left of
            // its parent menu, if the pop-up would otherwise go outside the
            // right edge of the window
            var global_pos_alt = refItem.mapToItem(
                appWindow.contentItem,
                altRightEdge,
                refY
                )

            if ((global_pos.x+menu.width) > appWindow.width) {
                global_pos.x = global_pos_alt.x-menu.width
            }
        }
        global_pos.x = Math.min(appWindow.contentItem.width-menu.width, global_pos.x)
        global_pos.y = Math.min(appWindow.contentItem.height-menu.height, global_pos.y)

        var parent_pos = appWindow.contentItem.mapToItem(
            menu.parent,
            global_pos
            )

        menu.x = parent_pos.x
        menu.y = parent_pos.y
        menu.visible = true

    }

    function checkMenuYPosition(refItem, h, refX, refY) {
        var ypos = refItem.mapToItem(
            appWindow.contentItem,
            refX,
            refY
            ).y
        return Math.max((ypos+h)-appWindow.contentItem.height, 0)
    }

    function maxMenuHeight(origMenuHeight) {
        // limit the maximum menu size to 80% of the height of the window
        // itself as long as the menu is less than 66% of the window height.
        // This meeans that you don't get a menu that is shortened by 2 pixels
        if (origMenuHeight > appWindow.contentItem.height*0.8) {
            return appWindow.contentItem.height*0.6
        }
        return origMenuHeight
    }

    onXChanged: {
        ui_layouts_model.set(ui_layouts_model.root_index, x, "position_x")
    }

    onYChanged: {
        ui_layouts_model.set(ui_layouts_model.root_index, y, "position_y")
    }

    onWidthChanged: {
        ui_layouts_model.set(ui_layouts_model.root_index, width, "width")
    }

    onHeightChanged: {
        ui_layouts_model.set(ui_layouts_model.root_index, height, "height")
    }

    property var preFullScreenVis: [x, y, width, height]
    property var qmlWindowRef: Window  // so javascript can reference Window enums.

    property bool fullscreen: false

    onFullscreenChanged : {
        if (fullscreen && visibility !== Window.FullScreen) {
            preFullScreenVis = [x, y, width, height]
            showFullScreen();
        } else if (!fullscreen) {
            visibility = qmlWindowRef.Windowed
            x = preFullScreenVis[0]
            y = preFullScreenVis[1]
            width = preFullScreenVis[2]
            height = preFullScreenVis[3]
        }
    }

    /*****************************************************************
     *
     * This captures any keyboard input and forwards to the last viewport
     * to become visible or the last viewport which the mouse has entered.
     * Hotkeys are all handled by the viewport
     *
     ****************************************************************/
    XsHotkeyArea {
        anchors.fill: parent
        context: "applicationWindow"
        focus: true
    }

    XsHotkey {
        id: fullscreen_hotkey
        sequence: "Ctrl+F"
        name: "Full Screen"
        description: "Makes the window go fullscreen"
        context: "any"
        onActivated: {
            appWindow.fullscreen = !appWindow.fullscreen
        }
    }

    XsHotkey {
        id: presentation_mode_hotkey
        sequence: "Tab"
        name: "Presentation Mode"
        description: "Toggles in/out of presentation mode"
        context: "any"
        onActivated: {
            appWindow.togglePresentationMode()
        }
    }

    Repeater {
        model: ui_layouts_model.numLayouts
        Item{
            XsHotkey {
                sequence: "F"+(ui_layouts_model.numLayouts-index)
                name: "Open Layout " + ui_layouts_model.get(ui_layouts_model.index(index, 0, ui_layouts_model.root_index), "layout_name")
                description: "Switches to the specified layout"
                context: "any"
                onActivated: {
                    layoutBar.selected_layout = index
                }
            }
        }
    }

    property alias presentation_mode_hotkey: presentation_mode_hotkey
    property alias fullscreen_hotkey: fullscreen_hotkey

    /*****************************************************************
     *
     * This item provides methods for simple dialogs. We instantiate
     * it here and make is available in the context so we can easily
     * make standard dialogs wherever we want
     *
     ****************************************************************/
    XsDialogHelpers {
        id: dialogHelpers
        z: -1000
        anchors.fill: parent
    }
    property alias dialogHelpers: dialogHelpers

    /*****************************************************************
     *
     * The sessionData entry is THE entry point to xstudio's backend
     * data. Most of the session state can be reached through this
     * 2D tree of data. Developers need to know about how QAbstractItemModel
     * c++ class works in the QML layer to understand how we index
     * into the sessionData object and use it as a model at different
     * levels to instance UI elements to display all information about playlists,
     * media, timelines, playheads and so-on.
     *
     ****************************************************************/
    XsSessionData {
        id: sessionData
        sessionActorAddr: studio.sessionActorAddr
    }

    XsConformTool {
        id: conform_tool
    }

    // Makes these important global items visible in child contexts
    property alias conformTool: conform_tool
    property alias theSessionData: sessionData.session
    property alias sessionSelectionModel: sessionData.sessionSelectionModel
    property alias globalStoreModel: sessionData.globalStoreModel
    property alias mediaSelectionModel: sessionData.mediaSelectionModel
    property alias playlistsModelData: sessionData.playlistsModelData
    property alias viewedMediaSetIndex: sessionData.viewedMediaSetIndex
    property alias selectedMediaSetIndex: sessionData.selectedMediaSetIndex
    property alias viewedMediaSetProperties: sessionData.viewedMediaSetProperties
    property alias selectedMediaSetProperties: sessionData.selectedMediaSetProperties
    property alias currentPlayheadData: sessionData.current_playhead_data
    property alias currentPlayheadUuid: sessionData.currentPlayheadUuid
    property alias onScreenMediaUuid: sessionData.onScreenMediaUuid
    property alias callbackTimer: sessionData.callbackTimer
    property alias viewsModel: sessionData.viewsModel
    property alias sessionProperties: sessionData.sessionProperties
    property alias bookmarkModel: sessionData.bookmarkModel
    property alias currentOnScreenMediaData: sessionData.currentOnScreenMediaData
    property alias currentOnScreenMediaSourceData: sessionData.currentOnScreenMediaSourceData
    property alias uiLayoutsModel: ui_layouts_model
    property alias singletonsModel: sessionData.singletonsModel
    property alias playheadKeySubplayheadIndex: sessionData.playheadKeySubplayheadIndex

    property var child_dividers: []

    property var layouts_model
    property var layouts_model_root_index

    ColumnLayout {

        anchors.fill: parent
        spacing: 1

        RowLayout{
            Layout.fillWidth: true
            spacing: 1

            XsMainMenuBar { id: menuBar
                Layout.preferredWidth: parent.width*1.85/3
            }
            XsLayoutModeBar {
                id: layoutBar
                layouts_model: ui_layouts_model
                layouts_model_root_index: ui_layouts_model.root_index
                Layout.fillWidth: true
            }
        }

        Item {

            Layout.fillWidth: true
            Layout.fillHeight: true

            // This repeater actually instanciates the layouts.
            Repeater {
                model: layouts_top_level
            }
        }
    }

    // this DelegateModel is set-up to load the layouts. A layout is only 
    // loaded when it becomes selected for the first time.
    DelegateModel {

        id: layouts_top_level
        model: ui_layouts_model
        rootIndex: ui_layouts_model.root_index

        delegate:
            Loader {
                anchors.fill: parent
                Component {
                    id: panel_component
                    XsSplitPanel {
                        id: root_panel
                        panels_layout_model: ui_layouts_model
                        panels_layout_model_index: ui_layouts_model.index(index, 0, ui_layouts_model.root_index)
                        visible: panels_layout_model_index == layoutBar.current_layout_index
                    }
                }
                property var v: index == layoutBar.current_layout_index.row && !layoutBar.movingLayout
                visible: index == layoutBar.current_layout_index.row 
                onVChanged: {
                    if (v && layout_name != undefined && sourceComponent == undefined) {
                        // load the panel
                        sourceComponent = panel_component
                    }
                }

            }
    }


    Component.onCompleted: {

        viewsModel.register_view("qrc:/views/playlists/XsPlaylists.qml", "Playlists")
        viewsModel.register_view("qrc:/views/media/XsMedialist.qml", "Media")
        viewsModel.register_view("qrc:/views/viewport/XsViewport.qml", "Viewport")
        viewsModel.register_view("qrc:/views/timeline/XsTimelinePanel.qml", "Timeline")
        viewsModel.register_view("qrc:/views/notes/XsNotesView.qml", "Notes")

        singletonsModel.register_singleton_qml("qrc:/views/notes/XsNotesSingleton.qml")
    }

    // here we instance 'singleton' items that may or may not be provided
    // with the 'views'. A view might want to fiddle with and/or add to
    // the menu models to inject menu items in menus other than its local
    // ones. Similarly a view might want to declare its hotkeys just once
    // so that they can be invoked at the app level without each instance
    // of a view being involved.
    // The place to do this is in its singleton item as doing it in views is
    // problematic as a there may be multiple instances of any given view.
    ListView {
        model: DelegateModel {
            model: singletonsModel
            delegate: Item {
                id: parent_item
                property var dynamic_widget
                Component.onCompleted: {
                    if (source.endsWith(".qml")) {
                        let component = Qt.createComponent(source)
                        if (component.status == Component.Ready) {
                            dynamic_widget = component.createObject(
                                parent_item, {})
                        } else {
                            console.log("Couldn't load singleton qml file.", component, component.errorString())
                        }
                    } else {
                        dynamic_widget = Qt.createQmlObject(source, parent_item)
                    }
                }
            }
        }
    }
    /*****************************************************************
     *
     * 'Pop-out' viewer management
     *
     ****************************************************************/

    Component {
        id: popout
        XsPopoutViewerWindow {
            palette: appWindow.palette
        }
    }

    Loader {
        id: popout_loader
    }

    function toggle_popout_viewer() {
        if (popout_loader.sourceComponent == undefined) {
            popout_loader.sourceComponent = popout
            popout_loader.item.visible = true
        } else {
            popout_loader.item.visible = !popout_loader.item.visible
        }
    }

    property bool popoutIsOpen: popout_loader.item ? popout_loader.item.visible : false
    property bool isPopoutViewer: false

    // to manage quickview windows
    XsQuickViewLauncher {}

}

