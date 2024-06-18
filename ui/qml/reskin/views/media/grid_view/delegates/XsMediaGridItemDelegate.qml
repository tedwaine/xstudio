// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtGraphicalEffects 1.15
import QtQml.Models 2.14
import xStudioReskin 1.0

import xstudio.qml.helpers 1.0
import "."

Rectangle{

    id: thumbnailDiv
    width: 180
    height: width / (16/9) //to keep 16:9 ratio  
    color: "transparent"
    property bool showBorder: false

    XsText{
        text: "No\nImage"
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        visible: thumbnailImageRole == undefined
    }

    XsImagePainter {
        id: thumbnailImgDiv
        anchors.fill: parent
        image: thumbnailImageRole
    }

    Component {
        id: highlight
        Rectangle {
            anchors.fill: parent
            z: 100
            border.width: 5
            border.color: palette.highlight
            color: "transparent"
        }    
    }

    Loader {
        id: loader
        anchors.fill: thumbnailImgDiv
    }

    onShowBorderChanged: {
        if (showBorder) loader.sourceComponent = highlight
        else loader.sourceComponent = undefined
    }

    // Rectangle{visible: isActive && !activeStateOnIndex; anchors.fill: parent; color: "transparent"; border.width: 2; border.color: borderColorHovered}


}