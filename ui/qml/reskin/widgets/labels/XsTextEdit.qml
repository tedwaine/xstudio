// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15

import xStudioReskin 1.0

TextEdit {
	id: widget

    property string hint: "Enter text here..."
    property color hintColor: XsStyleSheet.hintColor
    property color textColorNormal: palette.text
    property color textColorSelected: "black" 
    property color highlightColor: palette.highlight

    color: textColorNormal
    selectedTextColor: textColorSelected
    selectionColor: highlightColor

    selectByMouse: true
    wrapMode: TextEdit.Wrap
	verticalAlignment: Qt.AlignTop
    horizontalAlignment: Qt.AlignLeft

	font {
        pixelSize: XsStyleSheet.fontSize
        family: XsStyleSheet.fontFamily
        hintingPreference: Font.PreferNoHinting
    }

    XsText {
        id: hintDiv
        
        visible: !widget.text
        text: hint
        color: hintColor

    	width: parent.width
    	height: parent.height
	    leftPadding: parent.leftPadding
	    rightPadding: parent.rightPadding
	    topPadding: parent.topPadding
	    bottomPadding: parent.topPadding

        verticalAlignment: Qt.AlignTop
        horizontalAlignment: Qt.AlignLeft
    
    }
}