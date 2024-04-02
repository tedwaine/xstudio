// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import "./widgets"

Item{

    id: panel

    anchors.fill: parent

    property color panelColor: XsStyleSheet.panelBgColor
    property color bgColorPressed: palette.highlight 
    property color bgColorNormal: "transparent"
    property color forcedBgColorNormal: bgColorNormal
    property color borderColorHovered: bgColorPressed
    property color borderColorNormal: "transparent"
    property real borderWidth: 1
    property bool isSubDivider: false

    property real textSize: XsStyleSheet.fontSize
    property var textFont: XsStyleSheet.fontFamily
    property color textColorNormal: palette.text
    property color hintColor: XsStyleSheet.hintColor

    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real secBtnWidth: XsStyleSheet.secondaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding

    XsGradientRectangle{
        anchors.fill: parent
    }
    
    ColumnLayout {
        
        id: titleDiv
        anchors.fill: parent
        anchors.margins: panelPadding
        
        RowLayout{

            x: panelPadding
            spacing: 1
            Layout.fillWidth: true

            XsPrimaryButton{ id: addPlaylistBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/add.svg"
                onClicked: {
                    var pos = mapToItem(panel, x+width/2, y+height/2)
                    showMenu(pos.x ,pos.y)
                }
            }

            XsPrimaryButton{ id: deleteBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/delete.svg"
            }
            // XsSearchButton{ id: searchBtn
            //     Layout.preferredWidth: isExpanded? btnWidth*6 : btnWidth
            //     Layout.preferredHeight: btnHeight
            //     isExpanded: false
            //     hint: "Search playlists..."
            // }
            Item{
                Layout.fillWidth: true
                Layout.preferredHeight: btnHeight
            }
            XsPrimaryButton{ id: morePlaylistBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/more_vert.svg"
            }
        }

        ColumnLayout{ 
        
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            XsPlaylistHeader{
    
                id: playlistHeader        
                Layout.fillWidth: true
                height: XsStyleSheet.widgetStdHeight
    
            }
                            
            XsPlaylistItems{
                       
                id: playlistItems
                Layout.fillWidth: true
                Layout.fillHeight: true 
                y: playlistHeader.height
                itemRowWidth: width
    
            }
    
        }    
        
    }
    
    Loader {
        id: menu_loader
    }

    Component {
        id: plusMenuComponent
        XsPlaylistPlusMenu {
        }
    }
                    
    function showMenu(mx, my) {
        if (menu_loader.item == undefined) {
            menu_loader.sourceComponent = plusMenuComponent
        }
        menu_loader.item.x = mx
        menu_loader.item.y = my
        menu_loader.item.visible = true    
    }
    
}