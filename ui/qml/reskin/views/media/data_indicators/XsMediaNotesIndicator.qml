// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQml.Models 2.14
import xStudioReskin 1.0
import QtQuick.Layouts 1.15

Item{ 
    
    id: notesDiv
    clip: true

    property var gotBookmark: false
    property var gotBookmarkAnnotation: false
    property var gotBookmarkGrade: false
    property var gotBookmarkTransform: false
    property var gotBookmarkUuids_: bookmarkUuids

    onGotBookmarkUuids_Changed: {
        var bm = false
        var anno = false
        if (bookmarkUuids && bookmarkUuids.length) {
            bm = true
            for (var i in bookmarkUuids) {
                var idx = bookmarkModel.searchRecursive(bookmarkUuids[i], "uuidRole")
                if (idx.valid) {                    
                    if (bookmarkModel.get(idx,"hasAnnotationRole")) {
                        anno = true
                    }
                }
            }
        }
        gotBookmark = bm
        gotBookmarkAnnotation = anno
    }

    RowLayout{ id: attachmentIconsRow
        anchors.left: parent.left
        anchors.leftMargin: itemPadding*2
        anchors.verticalCenter: parent.verticalCenter
        spacing: itemPadding/2

        // TODO: add notes/annotation/colour/transform status data to
        // MediaItem in the main data model to drive these buttons ...
        
        XsSecondaryButton { 
                
            enabled: false

            Layout.minimumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.minimumHeight: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumHeight: XsStyleSheet.secondaryButtonStdWidth
            imgSrc: "qrc:/icons/sticky_note.svg"
            isColoured: gotBookmark
            onlyVisualyEnabled: gotBookmark
        }

        XsSecondaryButton { 
                
            enabled: false

            Layout.minimumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.minimumHeight: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumHeight: XsStyleSheet.secondaryButtonStdWidth
            imgSrc: "qrc:/icons/brush.svg"
            isColoured: gotBookmarkAnnotation
            onlyVisualyEnabled: gotBookmarkAnnotation

        }

        XsSecondaryButton { 
                
            enabled: false

            Layout.minimumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.minimumHeight: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumHeight: XsStyleSheet.secondaryButtonStdWidth
            imgSrc: "qrc:/icons/tune.svg"
            isColoured: gotBookmarkGrade
            onlyVisualyEnabled: gotBookmarkGrade

        }

        XsSecondaryButton { 
                
            enabled: false
            Layout.minimumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumWidth: XsStyleSheet.secondaryButtonStdWidth
            Layout.minimumHeight: XsStyleSheet.secondaryButtonStdWidth
            Layout.maximumHeight: XsStyleSheet.secondaryButtonStdWidth
            imgSrc: "qrc:/icons/open_with.svg"
            isColoured: gotBookmarkTransform
            onlyVisualyEnabled: gotBookmarkTransform

        }

    }
    Rectangle{
        width: headerThumbWidth; 
        height: parent.height
        anchors.right: parent.right
        color: bgColorPressed
    }

}