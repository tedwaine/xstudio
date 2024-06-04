// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xStudioReskin 1.0

import xstudio.qml.session 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0
import "./widgets"

Item{

    id: panel
    anchors.fill: parent
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
    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding

    property real rowHeight: XsStyleSheet.widgetStdHeight

    //#TODO: just for testing
    property bool highlightTextOnActive: false

    property alias mediaSelection: mediaList.selection

    XsGradientRectangle{
        anchors.fill: parent
    }

    ColumnLayout {

        anchors.fill: parent
        anchors.margins: panelPadding
        spacing: panelPadding

        RowLayout{

            spacing: 1
            Layout.fillWidth: true

            XsPrimaryButton{ id: addBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                Layout.alignment: Qt.AlignLeft
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
                onClicked: {
                    mediaList.deleteSelected()
                }
            }
            XsSearchButton{ id: searchBtn
                Layout.preferredWidth: isExpanded? btnWidth*6 : btnWidth
                Layout.preferredHeight: btnHeight
                isExpanded: false
                hint: "Search media..."
            }
            XsText{
                Layout.fillWidth: true
                Layout.minimumWidth: 0//btnWidth
                Layout.preferredHeight: btnHeight
                text: searchBtn.isExpanded? "" : inspectedMediaSetProperties.values.nameRole ? inspectedMediaSetProperties.values.nameRole : ""
                font.bold: true
                elide: Text.ElideRight

                opacity: searchBtn.isExpanded? 0:1
                Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.OutQuart }  }
            }
            XsPrimaryButton{ id: listViewBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/list.svg"
                isActive: true
                onClicked:{
                    listViewBtn.isActive = true
                    gridViewBtn.isActive = false
                }
            }
            XsPrimaryButton{ id: gridViewBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/view_grid.svg"
                enabled: false
                // onClicked:{
                //     listViewBtn.isActive = false
                //     gridViewBtn.isActive = true
                // }
                onPressed: {
                    listViewBtn.isActive = false
                    gridViewBtn.isActive = true
                }
                onReleased:{
                    listViewBtn.isActive = true
                    gridViewBtn.isActive = false
                }
            }
            XsPrimaryButton{ id: moreBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                Layout.alignment: Qt.AlignRight
                imgSrc: "qrc:/icons/more_vert.svg"
            }

        }

        XsMediaHeader{

            id: titleBar
            Layout.fillWidth: true
            height: XsStyleSheet.widgetStdHeight
        }

        XsMediaItems{

            id: mediaList
            Layout.fillWidth: true
            Layout.fillHeight: true

            y: titleBar.height

            columns_model_index: titleBar.columns_model_index
            itemRowHeight: rowHeight
            itemRowWidth: width

        }

    }

    function sort_media(media_list_column_index, ascending) {
        var col = media_list_column_index.row
        var media_list_idx = media_list_column_index.parent.row
        theSessionData.sortByMediaDisplayInfo(
            inspectedMediaSetIndex,
            media_list_idx,
            col,
            ascending)
    }

    Loader {
        id: menu_loader
    }

    Component {
        id: plusMenuComponent
        XsMediaListPlusMenu {
        }
    }
                    
    function showMenu(mx, my) {
        if (menu_loader.item == undefined) {
            menu_loader.sourceComponent = plusMenuComponent
        }
        repositionPopupMenu(
            menu_loader.item,
            panel,
            mx,
            my);
    }
}