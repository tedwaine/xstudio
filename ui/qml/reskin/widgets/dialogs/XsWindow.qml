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
    palette.highlight: XsStyleSheet.accentColor //== "#666666" ? Qt.lighter(XsStyleSheet.accentColor, 1.5) : XsStyleSheet.accentColor
    palette.text: XsStyleSheet.primaryTextColor
    palette.buttonText: XsStyleSheet.primaryTextColor
    palette.windowText: XsStyleSheet.primaryTextColor
    palette.button: Qt.darker("#414141", 2.4)
    palette.light: "#bb7700"
    palette.highlightedText: Qt.darker("#414141", 2.0)
    palette.brightText: "#bb7700"

}


