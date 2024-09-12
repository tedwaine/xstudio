import QtQuick 2.12
import QtQuick.Layouts 1.15

import xStudio 1.0
import xstudio.qml.viewport 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

import "./widgets"

Rectangle{
    id: viewportWidget

    color: "transparent"
    anchors.fill: parent
    property color gradient_colour_top: "#5C5C5C"
    property color gradient_colour_bottom: "#474747"

    property var view: viewLayout
    focus: true

    Item {
        anchors.fill: parent
        focus: true
        Keys.forwardTo: view
    }

    property real panelPadding: elementsVisible || tabBarVisible ||
        actionBarVisible || infoBarVisible ||
        toolBarVisible || transportBarVisible ? XsStyleSheet.panelPadding : 0

    Behavior on panelPadding {NumberAnimation{ duration: 150 }}

    XsGradientRectangle{
        anchors.fill: actionBar
    }
    XsViewportActionBar{
        id: actionBar
        anchors.top: parent.top
        actionbar_model_data_name: view.name + "_actionbar"
        opacity: elementsVisible && actionBarVisible ? 1 : 0
        showContextMenu: viewport.showContextMenu

        visible: opacity != 0.0
        Behavior on opacity {NumberAnimation{ duration: 150 }}

    }

    XsGradientRectangle{
        anchors.top: infoBar.top
        anchors.bottom: infoBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        topPosition: -y/height
        bottomPosition: parent.height/height + topPosition
    }
    XsViewportInfoBar{
        id: infoBar
        anchors.top: actionBar.bottom
        opacity: elementsVisible && infoBarVisible ? 1 : 0

        visible: opacity != 0.0
        Behavior on opacity {NumberAnimation{ duration: 150 }}
    }

    property color gradient_dark: "black"
    property color gradient_light: "white"

    XsPlayhead {
        id: viewportPlayhead
        uuid: view.playheadUuid
    }

    property alias viewportPlayhead: viewportPlayhead

    /*
    // Leaving this here for posterity. Uncommenting this will result in
    // debug spam telling you about data in viewportPlayheadDataModel.
    // It gives you an idea about what 'XsModuleData' data does.
    XsModuleData {
        id: viewportPlayheadDataModel

        // each playhead exposes its attributes in a model named after the
        // playhead UUID. We connect to the model like this:
        modelDataName: viewportPlayhead.uuid
    }
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

    property var visible_doc_widgets: []
    property var userData: user_data
    onUserDataChanged: {
        // update list of all visible docakble widgets
        if (userData == undefined) return
        var v = []
        for (var i = 0; i < userData.length; ++i) {
            if (userData[i][1]) {
                v.push(userData[i][0])
            }
        }
        visible_doc_widgets = v
    }

    // onUserDataChanged:{
    //     if(user_data == undefined){
    //         user_data = {"tabBar": true, "actionBar": true, "infoBar": true, "toolBar": true, "transportBar"; true}
    //     }
    // }

    function toggle_dockable_widget(widget_name) {

        // 'user_data' is model data attached to the panel instance. We can
        // put an array of data into user_data that tells us which dockable
        // toolbars are visible and where they are (left, right, top bottom.)
        if (user_data == undefined || typeof user_data == "string") {
            user_data = []
        }

        var u = user_data
        for (var i = 0; i < u.length; ++i) {
            var w = u[i]
            if (u[i][0] == widget_name) {
                // found a match. toggle visibility
                u[i][1] = !u[i][1]
                user_data = u
                return
            }
        }
        var new_entry = [widget_name, true, "left"]
        u.push(new_entry)
        // on setting user_data with the changes, we then rebuild the arrays
        // that drive the creation of the docking toolbars
        user_data = u

    }

    function move_dockable_widget(widget_name, placement) {

        var u = user_data
        for (var i = 0; i < u.length; ++i) {
            var w = u[i]
            if (u[i][0] == widget_name) {
                // found a match. toggle visibility
                u[i][2] = placement
                user_data = u
                return
            }
        }

    }

    XsLabel {
        text: "Media Not Found"
        color: XsStyleSheet.hintColor
        anchors.centerIn: parent
        font.pixelSize: XsStyleSheet.fontSize*1.2
        font.weight: Font.Medium
        visible: false //#TODO
    }

    RowLayout {

        id: viewLayout
        anchors.top: infoBar.bottom
        anchors.bottom: toolBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        XsLeftRightDockedTools {
            Layout.fillHeight: true
            dockedWidgetsModel: user_data
            Layout.preferredWidth: contentItem.childrenRect.width
        }


        ColumnLayout {
            spacing: 0

            XsTopBottomDockedTools {
                Layout.fillWidth: true
                dockedWidgetsModel: user_data
                Layout.preferredHeight: contentItem.childrenRect.height
                placement: "top"
            }

            XsViewport { id: viewport
                z: -100
                Layout.fillWidth: true
                Layout.fillHeight: true
                Component.onCompleted: {
                    viewportWidget.view = viewport
                }

            }

            XsTopBottomDockedTools {
                Layout.fillWidth: true
                dockedWidgetsModel: user_data
                Layout.preferredHeight: contentItem.childrenRect.height
                placement: "bottom"
            }
        }

        XsLeftRightDockedTools {
            Layout.fillHeight: true
            dockedWidgetsModel: user_data
            placement: "right"
            Layout.preferredWidth: contentItem.childrenRect.width
        }

    }

    XsGradientRectangle{
        anchors.fill: toolBar
    }
    XsViewportToolBar{
        id: toolBar
        anchors.bottom: transportBar.top
        toolbar_model_data_name: view.name + "_toolbar"
        opacity: elementsVisible && toolBarVisible ? 1 : 0

        visible: opacity != 0.0
        Behavior on opacity {NumberAnimation{ duration: 150 }}
    }

    XsGradientRectangle{
        anchors.fill: transportBar
    }
    XsViewportTransportBar {
        id: transportBar
        anchors.bottom: parent.bottom
        opacity: elementsVisible && transportBarVisible ? 1 : 0

        visible: opacity != 0.0
        Behavior on opacity {NumberAnimation{ duration: 150 }}
    }

    property bool elementsVisible: true
    onElementsVisibleChanged: {
        if(currentLayout == "Present") {
            menuBarVisible = elementsVisible
        }

        tabBarVisible = elementsVisible
    }
    property var currentLayout: isPopoutViewer? "popout" : appWindow.layoutName
    onCurrentLayoutChanged:{
        if (currentLayout !== "Present") {
            menuBarVisible = true
        }
        // else{
        //     menuBarVisible = elementsVisible
        // }
    }
    
    property bool menuBarVisible: true
    onMenuBarVisibleChanged:{
        appWindow.set_menu_bar_visibility(viewportWidget.menuBarVisible)
    }
    property bool tabBarVisible: true
    onTabBarVisibleChanged:{
        if (typeof forceHideTabs !== "undefined") {
            forceHideTabs = !tabBarVisible
        }
    }
    property bool actionBarVisible: true
    property bool infoBarVisible: true
    property bool toolBarVisible: true
    property bool transportBarVisible: true

}