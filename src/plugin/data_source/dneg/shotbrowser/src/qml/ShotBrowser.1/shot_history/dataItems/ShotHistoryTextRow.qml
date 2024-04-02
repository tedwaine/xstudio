// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

Rectangle{

    property alias text: textDiv.text
    property var bgColor: bgColorNormal
    property var textColor: hintColor

    property alias textDiv: textDiv
    
    color: bgColor

    XsText{ id: textDiv
        text: ""
        color: textColor
        height: parent.height
        anchors.verticalCenter: parent.verticalCenter
        
        Component.onCompleted: {
            if(width != parent.width){
                leftPadding = panelPadding
            }
        }
    }
}