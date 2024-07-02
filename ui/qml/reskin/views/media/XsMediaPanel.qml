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
import "./list_view"
import "./grid_view"
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
    property string mediaListSearchString

    property real textSize: XsStyleSheet.fontSize
    property var textFont: XsStyleSheet.fontFamily
    property color textColorNormal: palette.text
    property color hintColor: XsStyleSheet.hintColor

    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding

    property real rowHeight: XsStyleSheet.widgetStdHeight
    property real gridCellSize: 200

    //#TODO: just for testing
    property bool highlightTextOnActive: false

    property var mediaSelection: mediaSelectionModel.selectedIndexes

    property bool is_list_view: true

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
            XsSearchButton{ 
                id: searchBtn
                Layout.preferredWidth: isExpanded? btnWidth*6 : btnWidth
                Layout.preferredHeight: btnHeight
                isExpanded: false
                hint: "Search media..."
                onTextChanged: {
                    mediaListSearchString = text
                }
            }

            Item{
                Layout.fillWidth: true
                Layout.preferredHeight: btnHeight
            }
            XsComboBox{ id: sortValueDiv
                Layout.minimumWidth: btnWidth
                Layout.preferredWidth: btnWidth*3
                Layout.maximumWidth: btnWidth*3
                Layout.fillWidth: true
                Layout.preferredHeight: btnHeight
                visible: !is_list_view
    
                onCurrentIndexChanged: {
                }
            }
            XsPrimaryButton{ id: sortOrderBtn
                Layout.preferredWidth: btnWidth/2
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/trending.svg"
                imageDiv.rotation: isAsc? 90:270
                visible: !is_list_view

                property bool isAsc: true

                onClicked: { 
                    sortOrderBtn.isAsc = !sortOrderBtn.isAsc
                }
            }
            
            
            Item{
                Layout.fillWidth: true
                Layout.preferredHeight: btnHeight
            }
            // XsText{
            //     Layout.fillWidth: true
            //     Layout.minimumWidth: 0//btnWidth
            //     Layout.preferredHeight: btnHeight
            //     text: searchBtn.isExpanded? "" : inspectedMediaSetProperties.values.nameRole ? inspectedMediaSetProperties.values.nameRole : ""
            //     font.bold: true
            //     elide: Text.ElideRight
            //     opacity: searchBtn.isExpanded? 0:1
            //     Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.OutQuart }  }
            // }
            
            // XsSlider{
            //     visible: !is_list_view
            //     Layout.minimumWidth: btnWidth
            //     Layout.preferredWidth: btnWidth*2
            //     Layout.preferredHeight: btnHeight
            //     snapMode: Slider.NoSnap 
            //     stepSize: .1
            //     orientation: Qt.Horizontal
            //     // fillColor: muted? Qt.darker(palette.highlight,2) : palette.highlight
            //     // handleColor: muted? Qt.darker(palette.text,1.2) : palette.text
            //     value: gridCellSize
            //     from: 100
            //     to: 300
            //     onValueChanged: {
            //         console.log("gridCellSize: ", gridCellSize)
            //         gridCellSize = value
            //     }
            //     onReleased:{
            //     }
            // }

            XsIntegerValueControl {
                visible: !is_list_view

                Layout.minimumWidth: btnWidth
                Layout.preferredWidth: btnWidth*2
                Layout.maximumWidth: btnWidth*2
                Layout.preferredHeight: btnHeight
                Layout.maximumHeight: btnHeight

                text: "Scale"
                fromValue: 33 //100
                toValue: 100 //300
                defaultValue: 100 //50 //gridCellSize //200
                stepSize: 1
                valueText: parseFloat(value/100).toFixed(2)
                
                property real value: defaultValue
                onValueChanged:{
                    gridCellSize = value*3
                }
            }

            XsPrimaryButton{ id: gridViewBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/view_grid.svg"
                isActive: !is_list_view
                onPressed: {
                    is_list_view = false
                }
            }
            XsPrimaryButton{ 
                id: listViewBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                imgSrc: "qrc:/icons/list.svg"
                isActive: is_list_view
                onPressed: {
                    is_list_view = true
                }
            }
            XsPrimaryButton{ id: moreBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: btnHeight
                Layout.alignment: Qt.AlignRight
                imgSrc: "qrc:/icons/more_vert.svg"
            }

        }

        Loader {
            id: loader
            sourceComponent: is_list_view ? list_view : grid_view
            Layout.fillWidth: true
            Layout.fillHeight: true

            // onSourceComponentChanged: {
            // }
        }

        Component {
            id: list_view
            XsMediaListLayout {

                Layout.fillWidth: true
                Layout.fillHeight: true

            }
        }

        Component {
            id: grid_view
            XsMediaGridLayout {

                Layout.fillWidth: true
                Layout.fillHeight: true

                cellSize: gridCellSize

                Component.onCompleted: {
                    gridCellSize = cellSize
                }

            }
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