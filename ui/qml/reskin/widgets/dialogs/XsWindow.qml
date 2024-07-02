import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

import xStudioReskin 1.0

ApplicationWindow {
    id: widget

    flags: Qt.WindowStaysOnTopHint | Qt.Dialog
    color: XsStyleSheet.panelBgColor

    
    // override default palette
    palette.base: XsStyleSheet.panelBgColor
    palette.highlight: XsStyleSheet.accentColor
    palette.text: XsStyleSheet.primaryTextColor
    palette.buttonText: XsStyleSheet.primaryTextColor
    palette.windowText: XsStyleSheet.primaryTextColor
    palette.button: Qt.darker("#414141", 2.4)
    palette.light: "#bb7700"
    palette.highlightedText: Qt.darker("#414141", 2.0)
    palette.brightText: "#bb7700"

    property bool firstTimeShown: true

    onVisibleChanged: {
        if (firstTimeShown) {
            // try and position in the middle of the main session window
            if (typeof appWindow == "object") {
                x = appWindow.x + appWindow.width/2 - width/2
                y = appWindow.y + appWindow.height/2 - height/2
            }
            firstTimeShown = false
        }
    }
    Component.onCompleted: {
        appWindow
    }

    background: XsGradientRectangle{ 
        id: backgroundDiv
    }

}


