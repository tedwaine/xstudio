// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0

Rectangle{
    clip: true
    color: panelColor

    XsLabel {
        text: !queryRunning ? (presetsSelectionModel.hasSelection ? "No Results Found" : "Select a preset on left to view the results") : ""
        color: XsStyleSheet.hintColor
        visible: results.count == 0

        anchors.centerIn: parent
        width: parent.width - panelPadding*2
        height: parent.height - panelPadding*2

        font.pixelSize: XsStyleSheet.fontSize*1.2
        font.weight: Font.Medium
    }

    XsListView{ id: listDiv
        width: parent.width
        height: parent.height
        anchors.centerIn: parent

        property real panelPadding: XsStyleSheet.panelPadding
        property real listItemSpacing: panelPadding
        property real listItemWidth: width
        // property real listItemHeight: (XsStyleSheet.widgetStdHeight*3) + (1*3)

        model: DelegateModel { id: chooserModel
            property var notifyModel: results
            onNotifyModelChanged: model = notifyModel
            model: notifyModel

            delegate: DelegateChooser{ id: chooser
                role: "typeRole"

                DelegateChoice{
                    roleValue: "Version"

                      ShotHistoryListDelegate{
                        modelDepth: chooserModel.notifyModel.depthAtRow(index)
                        width: listDiv.listItemWidth
                        height: (XsStyleSheet.widgetStdHeight*4) + (4+1)
                        listSpacing: listDiv.listItemSpacing
                        delegateModel: chooserModel
                        popupMenu: versionResultPopup
                        groupingEnabled: resultsBaseModel.isGrouped
                    }
                }

                DelegateChoice{
                    roleValue: "Note"

                    NotesHistoryListDelegate{
                        width: listDiv.listItemWidth
                        height: (XsStyleSheet.widgetStdHeight*8) + (1*7)
                        listSpacing: listDiv.listItemSpacing
                        delegateModel: chooserModel
                        popupMenu: noteResultPopup
                    }
                }

                DelegateChoice{
                    roleValue: "Playlist"

                    XsSBRPlaylistViewDelegate{
                        width: listDiv.listItemWidth
                        height: ((XsStyleSheet.widgetStdHeight*4) + (4+1) )/1.8
                        delegateModel: chooserModel
                        popupMenu: playlistResultPopup
                    }
                }
            }

        }
    }
}