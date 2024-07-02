// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtGraphicalEffects 1.15
import QtQml.Models 2.14
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import xstudio.qml.helpers 1.0
import "."
// import "../../widgets"
import "../../common_delegates"

Rectangle{

    id: gridDelegate
    
    width: cellSize-20 //180
    height: width / (16/9) //to keep 16:9 ratio  
    color: isSelected? Qt.darker(palette.highlight, 2.75): XsStyleSheet.widgetBgNormalColor

    property real defaultCellSize: 180
    property real itemSpacing: 1
    property bool isSelected: false
    property bool isHovered: mouseArea.containsMouse || mArea.containsMouse
    property bool showBorder: false
    property bool showDetails: cellSize > 180

    border.color: palette.highlight
    border.width: isHovered? 1 : 0
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true 
        onClicked: {
            isSelected = !isSelected
        }
        onDoubleClicked: {
            showBorder = !showBorder
        }
    }

    ColumnLayout{
        anchors.fill: parent
        anchors.margins: itemSpacing
        spacing: itemSpacing

        RowLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: itemSpacing
            spacing: 0 //itemSpacing

            // Rectangle{ id: indicatorsDiv
            //     Layout.fillWidth: true
            //     // Layout.preferredWidth: visible? divHeight : 0
            //     // Layout.maximumWidth: visible? divHeight : 0
            //     Layout.preferredWidth: visible? iconsDiv.width : 0
            //     Layout.maximumWidth: visible? iconsDiv.width : 0
            
            //     Layout.fillHeight: true
            //     color: "transparent"
            //     visible: showDetails

            //     property real divHeight: (height- itemSpacing*3)/3 

            //     ColumnLayout{
            //         width: parent.width
            //         height: parent.hegiht - itemSpacing*2
            //         anchors.centerIn: parent
            //         spacing: itemSpacing
    
            //         property real divHeight: parent.divHeight

            //         // Item{ 
            //         //     Layout.fillWidth: true
            //         //     Layout.preferredHeight: parent.divHeight
            //         //     // color: "grey"
                        
            //         //     XsText{
            //         //         text: index //trackIndexRole
            //         //         anchors.centerIn: parent
            //         //         font.bold: true
            //         //     }
            //         // }

            //         XsPrimaryButton{ id: notesInd
            //             property bool hasNotes: false //noteCountRole == 0 ? false : true
            //             text: "N" //notes
            //             Layout.fillWidth: true
            //             Layout.preferredHeight: parent.divHeight
            //             font.pixelSize: textSize*1.2
            //             font.weight: hasNotes? Font.Bold:Font.Medium
            //             isUnClickable: true
            //             isActiveViaIndicator: false
            //             textDiv.color: hasNotes? palette.text : XsStyleSheet.hintColor
            //             enabled: false
            //             bgDiv.opacity: enabled? 1.0 : 0.5
            //             isActive: hasNotes
            //         }
            //         XsPrimaryButton{ id: dailiesInd
            //             property bool hasDailies: false //submittedToDailiesRole === undefined ? false :true
            //             text: "D" //dalies
            //             Layout.fillWidth: true
            //             Layout.preferredHeight: parent.divHeight
            //             font.pixelSize: textSize*1.2
            //             font.weight: hasDailies? Font.Bold:Font.Medium
            //             isUnClickable: true
            //             isActiveViaIndicator: false
            //             textDiv.color: hasDailies? palette.text : XsStyleSheet.hintColor
            //             enabled: false
            //             bgDiv.opacity: enabled? 1.0 : 0.5
            //             isActive: hasDailies
            //         }
            //         XsPrimaryButton{ id: clientInd
            //             property bool hasClient: false //dateSubmittedToClientRole === undefined ? false : true
            //             text: "C" //client
            //             Layout.fillWidth: true
            //             Layout.preferredHeight: parent.divHeight
            //             font.pixelSize: textSize*1.2
            //             font.weight: hasClient? Font.Bold:Font.Medium
            //             isUnClickable: true
            //             isActiveViaIndicator: false
            //             textDiv.color: hasClient? palette.text : XsStyleSheet.hintColor
            //             enabled: false
            //             bgDiv.opacity: enabled? 1.0 : 0.5
            //             isActive: hasClient
            //         }
    
            //     }
            // }

            Rectangle{ id: thumbDiv
                Layout.fillWidth:true
                Layout.fillHeight: true
                Layout.margins: showDetails? 0 : 1
                color: "transparent"
                border.width: 2
                border.color: XsStyleSheet.panelBgColor

                // XsText{
                //     text: "No\nImage"
                //     width: parent.width
                //     anchors.horizontalCenter: parent.horizontalCenter
                //     anchors.verticalCenter: parent.verticalCenter
                //     visible: thumbnailImageRole == undefined
                // } 
            
                // XsImagePainter {
                //     id: thumbnailImgDiv
                //     anchors.fill: parent
                //     image: thumbnailImageRole
                //     // z: -1
                // }

                // Loader {
                //     id: loader
                //     anchors.fill: thumbnailImgDiv
                // }

                XsMediaThumbnailImage { id: thumb
                    anchors.fill: parent
                    showBorder: gridDelegate.showBorder //isOnScreen
                    forcedHover: isHovered
                    highlightBorderThickness: 10

                    property bool playOnClick: false
                    property real itemPadding: XsStyleSheet.panelPadding/2
                    property bool isMissing
                    property bool isActive
                    property real mouseX
                    property real mouseY
                    property real headerThumbWidth

                        
                }
                XsText{
                    text: index+1 //selection_index //trackIndexRole
                    anchors.left: thumb.left
                    anchors.leftMargin: showDetails? 5 : 2.5
                    anchors.top: thumb.top
                    anchors.topMargin: showDetails? 5 : 1.5
                    font.bold: true
                    color: isSelected ? palette.highlight : palette.text 

                    layer.enabled: true
                    layer.effect: DropShadow{
                        verticalOffset: 1
                        horizontalOffset: 1
                        color: "#010101"
                        radius: 1
                        samples: 3
                        spread: 0.5
                    }
                }

            }

            Rectangle{ id: iconsDiv
                Layout.fillWidth: true
                Layout.preferredWidth: visible? divHeight : 0
                Layout.maximumWidth: visible? divHeight : 0
                Layout.fillHeight: true
                color: "transparent"
                visible: showDetails

                property real divHeight: (height- itemSpacing*4)/4 

                ColumnLayout{
                    anchors.fill: parent
                    spacing: itemSpacing

                    property real divHeight: parent.divHeight
    
                    XsSecondaryButton { 
                        enabled: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.divHeight
                        imgSrc: "qrc:/icons/sticky_note.svg"
                        // isColoured: gotBookmark
                        // onlyVisualyEnabled: gotBookmark
                    }
                    XsSecondaryButton { 
                        enabled: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.divHeight
                        imgSrc: "qrc:/icons/brush.svg"
                        // isColoured: gotBookmarkAnnotation
                        // onlyVisualyEnabled: gotBookmarkAnnotation
                    }
                    XsSecondaryButton { 
                        enabled: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.divHeight
                        imgSrc: "qrc:/icons/tune.svg"
                        // isColoured: gotBookmarkGrade
                        // onlyVisualyEnabled: gotBookmarkGrade
                    }
                    XsSecondaryButton { 
                        enabled: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.divHeight
                        imgSrc: "qrc:/icons/open_with.svg"
                        // isColoured: gotBookmarkTransform
                        // onlyVisualyEnabled: gotBookmarkTransform
                    }

                }
            }

        }

        XsText{ id: fileNameDiv
            text: nameRole
            elide: Text.ElideMiddle
            Layout.fillWidth: true
            Layout.minimumHeight: visible? divHeight/1.5 : 0
            Layout.preferredHeight: visible? divHeight/1.5 : 0
            Layout.maximumHeight: visible? divHeight/1.5 : 0
            // visible: cellSize > 140
            visible: showDetails

            property real divHeight: (btnHeight - 2)
            
            isHovered: mArea.containsMouse
            MouseArea{
                id: mArea
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
            }
        }

    }



    

    // Component {
    //     id: highlight

    //     XsMediaThumbnailHighlight {
    //         anchors.fill: parent
    //         z: 100
    //     }    
    // }
    // onShowBorderChanged: {
    //     if (showBorder) loader.sourceComponent = highlight
    //     else loader.sourceComponent = undefined
    // }
    // Rectangle{visible: isActive && !activeStateOnIndex; anchors.fill: parent; color: "transparent"; border.width: 2; border.color: borderColorHovered}


}