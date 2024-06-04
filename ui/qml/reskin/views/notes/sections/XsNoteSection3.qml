// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.bookmarks 1.0

Rectangle{
    color: bgColorNormal

    property bool isHovered: timeDiv.hovered || closeBtn.hovered || 
        authorDiv.hovered || 
        inSetDiv.hovered || outSetDiv.hovered || durLoopDiv.hovered || 
        noteTypeCombo.hovered || noteTypeCombo.popupOptions.opened
    
    property real minHeight: itemHeight - itemSpacing/2
    property real minWidth: 40

    GridLayout {
        anchors.fill: parent
        columnSpacing: 1
        rowSpacing: 1
        columns: 4
        // rows: 6

        /// row1 - Time
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor
        }
        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.minimumWidth: minWidth*2
            Layout.minimumHeight: minHeight
            color: panelColor

            XsTextInput{ id: timeDiv
                anchors.centerIn: parent
                color: highlightColor
                text: createdRole
                readOnly: true
            }
        }
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor

            XsSecondaryButton{ id: closeBtn
                anchors.fill: parent
                imgSrc: "qrc:/icons/close.svg"

                onClicked: { //#TODO: for testing
                    deleteNote(uuidRole)
                }
            }
        }

        /// row2 - Author
        Rectangle {
            Layout.columnSpan: 4
            Layout.fillWidth: true
            Layout.minimumHeight: minHeight
            color: panelColor

            XsTextInput { id: authorDiv
                anchors.centerIn: parent
                color: highlightColor
                text: authorRole
            }
        }

        /// row3 - In
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor
            
            XsText {
                anchors.fill: parent
                text: "In"
            }
        }
        Rectangle {
            color: panelColor
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.minimumWidth: minWidth*2
            Layout.minimumHeight: minHeight

            XsText {
                anchors.fill: parent
                anchors.margins: 1
                text: startTimecodeRole
            }
        }
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor

            XsSecondaryButton{ 
                id: inSetDiv
                anchors.fill: parent
                text: "Set"
                onClicked: {
                    if (ownerRole == currentPlayhead.mediaUuid) {
                        startFrameRole = currentPlayhead.mediaFrame
                    }
                }
            }
        }

        /// row4 - Out
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor
            
            XsText {
                anchors.fill: parent
                text: "Out"
            }
        }
        Rectangle {
            color: panelColor
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.minimumWidth: minWidth*2
            Layout.minimumHeight: minHeight

            XsText {
                anchors.fill: parent
                anchors.margins: 1
                text: endTimecodeRole
            }
        }
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor

            XsSecondaryButton{ 
                id: outSetDiv
                anchors.fill: parent
                text: "Set"
                onClicked: {
                    if (ownerRole == currentPlayhead.mediaUuid) {
                        endFrameRole = currentPlayhead.mediaFrame
                    }
                }

            }
        }

        /// row5 - Duration
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor
            
            XsText {
                anchors.fill: parent
                text: "Dur"
            }
        }
        Rectangle {
            color: panelColor
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.minimumWidth: minWidth*2
            Layout.minimumHeight: minHeight

            XsText { 
                anchors.fill: parent
                anchors.margins: 1
                text: durationTimecodeRole
            }
        }
        Rectangle {
            Layout.minimumWidth: minWidth
            Layout.minimumHeight: minHeight
            color: panelColor

            XsSecondaryButton{ id: durLoopDiv
                anchors.fill: parent
                text: "Loop"
            }
        }

        XsPreference {
            id: bookmark_categories_value
            index: globalStoreModel.searchRecursive("/core/bookmark/category", "pathRole")
        }    

        XsBookmarkCategories {
            id: bookmark_categories
            value: bookmark_categories_value.value
        }
    
        //row6 - type
        Rectangle {
            Layout.columnSpan: 4
            Layout.fillWidth: true
            Layout.minimumHeight: minHeight
            color: panelColor

            XsComboBox { 
                id: noteTypeCombo
                model: bookmark_categories
                width: parent.width
                height: minHeight
                textRole: "textRole"
                displayText: (categoryRole == "" || categoryRole == undefined) ? "Note Type" : categoryRole
                textColorNormal: popupOptions.opened? palette.text : categoryRole=="-"? "grey" : palette.text

                onActivated: {
                    let ind = model.index(index, 0)
                    let col = model.get(ind, "colourRole")
                    let cat = model.get(ind, "valueRole")
                    if(col == undefined)
                        col = ""
                    categoryRole = cat
                    colourRole = col
                }

                Rectangle {
                    anchors.fill: parent
                    color: colourRole != undefined ? colourRole : transparent
                    opacity: 0.3
                }
            }
        }

    }

}