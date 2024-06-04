// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtGraphicalEffects 1.15
import QtQml.Models 2.14
import xStudioReskin 1.0

import xstudio.qml.helpers 1.0
import "."

Rectangle{

    id: thumbnailDiv
    height: width / (16/9) //to keep 16:9 ratio  
    color: "transparent"
    property bool showBorder: false

    XsText{
        text: "No\nImage" //#TODO
        width: parent.width - itemPadding*2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.weight: isActive? Font.ExtraBold : Font.Normal
        visible: thumbnailImageRole == undefined && !isMissing
        color: hintColor
        //lineHeight: activeStateOnIndex? (width/100)*1.5 : .7 
        // font.pixelSize: XsStyleSheet.fontSize-2
    }

    XsImagePainter {
        id: thumbnailImgDiv
        anchors.fill: parent
        image: thumbnailImageRole
    }

    XsImage{ 
        anchors.centerIn: parent
        height: parent.height*0.85
        width: height
        source: "qrc:/icons/play_circle.svg"
        opacity: localHovered ? 1.0 : 0.75
        visible: hovered
        property var mx: mouseX-thumbnailDiv.parent.parent.x-x
        property var my: mouseY-thumbnailDiv.y-y
        property var localHovered: hovered ? (mx > 0 && mx < width && my > 0 && my < height) : false
        onLocalHoveredChanged: playOnClick = localHovered    
    }

    Component {
        id: highlight
        XsMediaThumbnailHighlight {
            anchors.fill: parent
            z: 100
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

    Rectangle{
        width: headerThumbWidth; 
        height: parent.height
        anchors.right: parent.right
        color: bgColorPressed
    }

}