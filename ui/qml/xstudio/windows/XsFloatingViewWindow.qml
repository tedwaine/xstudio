// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0
import QtQml.Models 2.14
import QtQuick.Layouts 1.4

import xStudio 1.0

XsWindow {

    id: floatingWindow
    title: view_name

    flags: Qt.WindowStaysOnTopHint

    property var name: view_name // 'view_name' provided by model
    property var content_qml: view_qml_source // 'view_qml_source' provided by model
    property var content_item

    onClosing: {
        window_is_visible = false
    }

    onWidthChanged: {
        if (content_item) content_item.width = width
        storeSizeAndPosition()
    }

    onHeightChanged: {
        if (content_item) content_item.height = height
        storeSizeAndPosition()
    }

    onXChanged: storeSizeAndPosition()
    onYChanged: storeSizeAndPosition()

    onVisibleChanged: {
        // use last positin/size numbers if we have any
        if (visible && floating_window_positions.hasOwnProperty(view_name)) {
            x = floating_window_positions[view_name].x
            y = floating_window_positions[view_name].y
            width = floating_window_positions[view_name].width
            height = floating_window_positions[view_name].height
        } else if (visible) {
            // for now, hardcode panel size
            x = 200
            y = 200
            width = 1024
            height = 400
        }

        if (content_item) content_item.visible = visible
    }

    function storeSizeAndPosition() {
        if(updateTimer.running) {
            updateTimer.restart()
        } else {
            updateTimer.start()
        }
    }

    Timer {
        id: updateTimer
        interval: 1000
        running: false
        repeat: false
        onTriggered: {

            // window gets re-positioned by QML when it is hidden, so don't
            // store if not visible
            if (!floatingWindow.visible) return
            // store window position and size - floating_window_positions
            // SHOULD be a JSON object.
            var p = {}
            p.x = x
            p.y = y
            p.width = width
            p.height = height
            var v = floating_window_positions
            if (typeof v != "object") {
                v = {}
            }
            v[view_name] = p
            floating_window_positions = v
        }
    }

    onContent_qmlChanged: {

        if (typeof content_qml != "string") return;

        if (content_qml.endsWith(".qml")) {

            let component = Qt.createComponent(content_qml)

            if (component.status == Component.Ready) {

                if (content_item != undefined) content_item.destroy()
                content_item = component.createObject(
                    floatingWindow,
                    {
                    })

                // identify as an XStudioPanel - this is used when menu items
                // are actioned to identify the 'panel' context of the parent
                // menu
            } else {
                console.log("Error loading panel:", component, component.errorString())
            }

        } else {

            content_item = Qt.createQmlObject(content_qml, floatingWindow)
            // see note above

        }

    }

}

