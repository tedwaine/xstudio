// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import Qt.labs.qmlmodels 1.0
import QtQml.Models 2.14

import xStudioReskin 1.0
import xstudio.qml.module 1.0
import xstudio.qml.viewport 1.0

Item{
    id: panel
    anchors.fill: parent

    property bool isTestMode: false

    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding
    property color panelColor: XsStyleSheet.panelBgColor

    XsGradientRectangle{ id: backgroundDiv
        anchors.fill: parent
    }
    
    Item{
        
        id: titleBar
        width: parent.width; 
        height: btnHeight+(panelPadding*2)
        clip: true
        
        RowLayout{

            x: panelPadding
            spacing: 1
            width: parent.width-(x*2)
            height: btnHeight
            anchors.verticalCenter: parent.verticalCenter

            XsPrimaryButton{ 
                
                id: addBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/add.svg"

                onClicked: {
                    addNote()
                }
            }

            XsPrimaryButton {

                id: deleteBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/delete.svg"

                onClicked: {
                }
            }
            XsText{
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredHeight: parent.height
                text: {
                    if (dropdownNoteScope.currentIndex == 0) {
                        return currentOnScreenMediaData.values.nameRole
                    } else if (dropdownNoteScope.currentIndex == 1) { 
                        return selectedMediaSetProperties.values.nameRole
                    } else {
                        return "All Session Notes"
                    }
                }
                font.bold: true
                id: theTitle
            }
            XsText{
                Layout.minimumWidth: 0
                Layout.preferredWidth: 50
                Layout.preferredHeight: parent.height
                text: "Scope:"
            }
            XsComboBox { id: dropdownNoteScope
                model: ["Media", "Playlist", "Session"]
                Layout.minimumWidth: 0
                Layout.preferredWidth: btnWidth*2.2
                Layout.preferredHeight: parent.height
            }
            XsPrimaryButton{ id: moreBtn
                Layout.minimumWidth: 0
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/more_vert.svg"
            }
        }
        
    }

    Rectangle{ 
        
        id: notesDiv
        x: panelPadding
        y: titleBar.height
        width: panel.width-(x*2)
        height: panel.height-y-panelPadding
        color: panelColor

        XsListView {
            
            id: list

            anchors.fill: parent

            property real listItemSpacing: panelPadding
            property real listItemWidth: width
            property real listItemHeight: 20 * 6 //6*notesDelegate.itemHeight

            model: DelegateModel { 
                id: chooserModel
                model: bookmarkFilterModel
                delegate: XsNotesItemDelegate{ 
                        
                    id: notesDelegate
                    width: list.listItemWidth
                    height: list.listItemHeight
                    listItemSpacing: list.listItemSpacing
                    isActive: index == list.currentIndex
                } 
            }

        }

    }

    property var mediaOrder: updateMediaOrder()

    property var playlistFollower: selectedMediaSetIndex
    onPlaylistFollowerChanged: {
        theTitle
    }
    function updateTitle() {
        theTitle
    }

    function updateMediaOrder() {

        // here we make a map of media index (from the current playlist) 
        // with the media uuid as the index. This is used by XsBookmarkFilterModel
        // to order the notes according to the media order in the playlist
        let result = {}
        if(selectedMediaSetIndex.valid) {

            let model = selectedMediaSetIndex.model

            // from playlist, to get to the media list within we go to first row/column
            let mediaind = selectedMediaSetIndex.model.index(0, 0, selectedMediaSetIndex)

            let count = model.rowCount(mediaind)
            for(let i=0;i<count;i++) {
                result[model.get(model.index(i,0,mediaind), "actorUuidRole")] = i
            }

        }
        return result
    }

    function addNote() {
        if(bookmarkModel.insertRows(bookmarkModel.rowCount(), 1)) {
            // set owner..
            let ind = bookmarkModel.index(bookmarkModel.rowCount()-1, 0)
            bookmarkModel.set(ind, currentOnScreenMediaData.values.actorUuidRole, "ownerRole")
            bookmarkModel.set(ind, currentPlayheadData.attributeRoleData("Position Seconds"), "startRole")
            bookmarkModel.set(ind, currentOnScreenMediaData.values.nameRole, "subjectRole")
            bookmarkModel.set(ind, 0, "durationRole")
            bookmarkModel.set(ind, preferences.note_category.value, "categoryRole")
            bookmarkModel.set(ind, preferences.note_colour.value, "colourRole")

        }
    }

    function deleteNote(uuid) {
        var idx = bookmarkModel.searchRecursive(uuid, "uuidRole")
        if(idx.valid) {
            bookmarkModel.removeRows(idx.row, 1, idx.parent)
        }
    }

    XsBookmarkFilterModel {
        id: bookmarkFilterModel
        sourceModel: bookmarkModel
        currentMedia: onScreenMediaUuid // this property is made visible by XsSessionWindow
        showHidden: true
        depth: dropdownNoteScope.currentIndex
        mediaOrder: panel.mediaOrder
    }

}