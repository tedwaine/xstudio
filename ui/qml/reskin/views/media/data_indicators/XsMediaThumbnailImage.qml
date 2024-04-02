// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQml.Models 2.14
import xStudioReskin 1.0

import xstudio.qml.helpers 1.0

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
        visible: mediaThumbnail == undefined && !isMissing
        color: hintColor
        //lineHeight: activeStateOnIndex? (width/100)*1.5 : .7 
        // font.pixelSize: XsStyleSheet.fontSize-2
    }

    XsImagePainter {
        id: thumbnailImgDiv
        anchors.fill: parent
        image: mediaThumbnail
    }

    /*XsImage{ 
        id: thumbnailImgDiv
        visible: source!=""
        clip: true
        anchors.fill: parent
        anchors.margins: 1
        fillMode: Image.PreserveAspectFit //isMissing ? Image.PreserveAspectFit : Image.Stretch 
        source: isMissing? "qrc:/icons/error.svg" : thumbnailURLRole ? thumbnailURLRole : "" //"qrc:/icons/theaters.svg"
        rotation: source=="qrc:/icons/theaters.svg"? 90 : 0
        imgOverlayColor: "transparent"//isMissing? errorColor : _thumbnail? "transparent" : hintColor
    }*/

    Rectangle {
        border.width: 2
        border.color: palette.highlight
        anchors.fill: thumbnailImgDiv
        color: "transparent"
        visible: showBorder 
    }

   // Rectangle{visible: isActive && !activeStateOnIndex; anchors.fill: parent; color: "transparent"; border.width: 2; border.color: borderColorHovered}

    Rectangle{
        width: headerThumbWidth; 
        height: parent.height
        anchors.right: parent.right
        color: bgColorPressed
    }

}