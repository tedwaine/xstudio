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
import xstudio.qml.viewport 1.0

Item{
    id: panel
    anchors.fill: parent

    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight+4
    property real panelPadding: XsStyleSheet.panelPadding
    property color panelColor: XsStyleSheet.panelBgColor

    XsGradientRectangle{ id: backgroundDiv
        anchors.fill: parent
    }
    
    XsPreference {
        id: note_category
        index: globalStoreModel.searchRecursive("/core/bookmark/note_category", "pathRole")
    }    
    property alias note_category: note_category.value

    XsPreference {
        id: note_colour
        index: globalStoreModel.searchRecursive("/core/bookmark/note_colour", "pathRole")
    }    
    property alias note_colour: note_colour.value

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
            Item{
                Layout.preferredWidth: 1
                Layout.preferredHeight: parent.height
            }

            XsPrimaryButton {
                id: previousBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/chevron_right.svg"
                imageDiv.rotation: 180
                enabled: chooserModel.count > 1 && list.currentIndex >0

                onClicked: {
                    list.currentIndex = list.currentIndex==0 ? 0 : list.currentIndex--
                }
            }
            XsPrimaryButton {
                id: nextBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/chevron_right.svg"
                enabled: chooserModel.count > 1 && list.currentIndex < chooserModel.count-1

                onClicked: {
                    list.currentIndex = list.currentIndex==chooserModel.count-1 ? chooserModel.count-1 : list.currentIndex++
                }
            }
            XsText{
                id: theTitle
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredHeight: parent.height
                text: {
                    if (dropdownNoteScope.currentIndex == 0) {
                        return currentOnScreenMediaData.values.nameRole? currentOnScreenMediaData.values.nameRole: "TBD"
                    } else if (dropdownNoteScope.currentIndex == 1) { 
                        return viewedMediaSetProperties.values.nameRole? viewedMediaSetProperties.values.nameRole: "TBD"
                    } else {
                        return "All Session Notes"
                    }
                }
                font.bold: true
                elide: Text.ElideMiddle
            }
            Rectangle{
                Layout.preferredWidth: 1
                Layout.preferredHeight: parent.height
                color: XsStyleSheet.menuBarColor
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

    property var playlistFollower: viewedMediaSetIndex
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
        if(viewedMediaSetIndex.valid) {

            let model = viewedMediaSetIndex.model

            // from playlist, to get to the media list within we go to first row/column
            let mediaind = viewedMediaSetIndex.model.index(0, 0, viewedMediaSetIndex)

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
            bookmarkModel.set(ind, currentPlayhead.positionSeconds, "startRole")
            bookmarkModel.set(ind, currentOnScreenMediaData.values.nameRole, "subjectRole")
            bookmarkModel.set(ind, 0, "durationRole")
            bookmarkModel.set(ind, note_category, "categoryRole")  //#TODO: preferences error
            bookmarkModel.set(ind, note_colour, "colourRole")

            list.currentIndex = chooserModel.count-1 //ind
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
        currentMedia: currentPlayhead.mediaUuid // this property is made visible by XsSessionWindow
        showHidden: true
        depth: dropdownNoteScope.currentIndex
        mediaOrder: panel.mediaOrder
        excludedCategories: ["Grading"]
    }

}