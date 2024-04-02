// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

import xStudioReskin 1.0

Rectangle{

    color: XsStyleSheet.panelTitleBarColor
    clip: true

    XsText{
        text: "Playlist ("+playlistItems.count+")"
        anchors.left: parent.left
        anchors.leftMargin: panelPadding
        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Text.AlignLeft
    }

    XsSecondaryButton{
        width: secBtnWidth
        height: secBtnWidth
        imgSrc: "qrc:/icons/filter_none.svg"
        anchors.right: errorBtn.left
        anchors.rightMargin: panelPadding
        anchors.verticalCenter: parent.verticalCenter
    }

    XsSecondaryButton{ id: errorBtn
        width: secBtnWidth
        height: secBtnWidth
        imgSrc: "qrc:/icons/error.svg"
        anchors.right: parent.right
        anchors.rightMargin: panelPadding + panelPadding/2
        anchors.verticalCenter: parent.verticalCenter
    }

}