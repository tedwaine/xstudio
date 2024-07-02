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
import xstudio.qml.embedded_python 1.0

Item{

    id: panel
    anchors.fill: parent

    Rectangle{
        anchors.fill: parent
        color: XsStyleSheet.panelBgColor
    }

    // XsPreference {
    //     id: pythonHistory
    //     path: "/ui/qml/python_history"
    // }

    property int itemHeight: 22
    property int itemSpacing: 1
    property int padding: 6

    property int buttonCount: 3
    property int totalLogCount: logsView.count
    onTotalLogCountChanged: statusText = "Status: Count updated."
    property int startSelection: -1
    property int endSelection: -1
    property int selectedCount: startSelection == -1 ? 0 : endSelection - startSelection + 1

    // property int showLevel: 5

    property string statusText

    function resetSelection(){
        startSelection= -1
        endSelection= -1
    }

    Rectangle{

        width: parent.width - padding*2
        height: parent.height - padding*2
        x: padding
        y: padding
        color: "transparent"

        ColumnLayout {
            anchors.fill: parent

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: itemHeight

                XsTextField {
                    Layout.fillWidth: true
                    Layout.minimumHeight: itemHeight
                    Layout.maximumHeight: itemHeight

                    placeholderText: "Enter filter text"
                    onTextEdited: {
                        logger.setFilterWildcard(text)
                        // totalLogCount = logger.rowCount()
                    }
                }

                XsComboBox { id: logLevel
                    // Layout.fillWidth: true
                    Layout.minimumHeight: itemHeight
                    Layout.maximumHeight: itemHeight
                    Layout.preferredWidth: 100
                    model: logger.logLevels
                    currentIndex: logger.logLevel
                    onCurrentIndexChanged: {
                        logger.logLevel = currentIndex
                        resetSelection()
                    }
                }
                // XsLabel {
                //     text: "Show on"
                // }

                // XsComboBox { id: showLogLevel
                //     // Layout.fillWidth: true
                //     Layout.minimumHeight: itemHeight
                //     Layout.maximumHeight: itemHeight
                //     Layout.preferredWidth: 100
                //     model: logger.logLevels
                //     currentIndex: showLevel
                //     onCurrentIndexChanged: {
                //         showLevel = currentIndex
                //     }
                // }
            }

            ListView {
                id: logsView
                Layout.fillWidth: true
                Layout.fillHeight: true

                // contentWidth: width*2
                flickableDirection: Flickable.AutoFlickDirection
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                ScrollBar.vertical: XsScrollBar {
                    policy: ScrollBar.AlwaysOn
                }
                ScrollBar.horizontal: ScrollBar {}
                model: logger

                delegate:
                Rectangle{
                    width: logsView.width
                    height: msg.contentHeight
                    color:  index >= startSelection && index <= endSelection ? XsStyleSheet.hintColor : Qt.darker("#222", index % 2 ? 1.0 :1.25)

                    required property string timeRole
                    required property int levelRole
                    required property string levelStringRole
                    required property string stringRole
                    required property int index
                    property color level_color: logsView.level_colour(levelRole)

                    MouseArea{
                        anchors.fill: parent
                        z: 1
                        onClicked: {
                            if (mouse.button == Qt.LeftButton) {
                                if (mouse.modifiers & Qt.ShiftModifier) {
                                    if(index < startSelection) {
                                        endSelection = startSelection
                                        startSelection = index
                                    } else {
                                        endSelection = index
                                    }
                                } else {
                                    startSelection = endSelection = index
                                }
                            }
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.rightMargin: 10
                        TextEdit {
                            id: msg
                            wrapMode: TextEdit.Wrap
                            readOnly: true
                            verticalAlignment: Text.AlignVCenter
                            color: level_color
                            text: stringRole
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        Text {
                            verticalAlignment: Text.AlignTop
                            color: level_color
                            text: "["+levelStringRole+"]"
                            Layout.fillHeight: true
                        }

                        Text {
                            verticalAlignment: Text.AlignTop
                            color: level_color
                            text: Qt.formatDateTime(timeRole, "hh:mm:ss")
                            Layout.fillHeight: true
                        }
                    }
                }


                function level_colour(level)
                {
                    if(level == 0) //trace
                    return "cyan"
                    if(level == 1) //debug
                    return "grey"
                    if(level == 2) // info
                    return "green"
                    if(level == 3) // warn
                    return "yellow"
                    if(level == 4) // error
                    return "red"
                    if(level == 5) // critical
                    return "purple"
                    if(level == 6) // off
                    return "black"
                }

            }

            Rectangle{id: countTexts
                Layout.fillWidth: true
                Layout.minimumHeight: itemHeight
                Layout.maximumHeight: itemHeight
                color: "transparent"


                Text { id: textStatus
                    anchors.left: parent.left
                    anchors.leftMargin: itemSpacing/2
                    color: "light grey"
                    elide: Text.ElideRight
                    width: parent.width-70

                    text: statusText
                    onTextChanged: if(statusText!="") opacity=1
                    opacity: 0
                    onOpacityChanged: if(opacity==0) statusText=""

                    Behavior on opacity { NumberAnimation { easing.type: Easing.InQuart; duration: 3000; from: 1; to: 0; onRunningChanged: if(!running) textStatus.opacity = 0 } }
                    Component.onCompleted: textStatus.opacity = 0
                }
                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: itemSpacing/2
                    color: "light grey"
                    text: "Shown: "+totalLogCount + " | Selected: " + selectedCount
                }
            }

            Rectangle{id: buttons
                Layout.fillWidth: true
                Layout.minimumHeight: itemHeight
                Layout.maximumHeight: itemHeight
                color: "transparent"

                Row{
                    width: parent.width
                    height: itemHeight
                    x: spacing/2
                    spacing: itemSpacing

                    XsPrimaryButton {
                        text: "Select All"
                        onClicked: {startSelection = 0; endSelection = logger.rowCount()-1}
                        width: parent.width/buttonCount - itemSpacing
                        height: itemHeight
                    }
                    XsPrimaryButton {
                        text: "Copy to Clipboard"
                        enabled: selectedCount
                        onClicked: {
                            logger.copyLogToClipboard(startSelection, selectedCount)
                            statusText = "Status: Copied to Clipboard."
                        }

                        width: parent.width/buttonCount - itemSpacing
                        height: itemHeight
                    }
                    XsPrimaryButton {
                        text: "Save to File"
                        onClicked: {
                            dialogHelpers.showFileDialog(
                                function(fileUrl, undefined, func) {
                                    if(fileUrl !== false) {
                                        var path = fileUrl.toString()
                                        var ext = path.split('.').pop()
                                        if(path == ext) {
                                            path = path + ".log"
                                        }
                                        logger.saveLogToFile(Qt.resolvedUrl(path), startSelection, selectedCount)
                                        statusText = "Status: Saved to "+path
                                    }
                                },
                                file_functions.defaultSessionFolder(),
                                "Save Log",
                                "log",
                                ["Log Files (*.log)"],
                                false,
                                false
                            )
                        }
                        enabled: selectedCount
                        width: parent.width/buttonCount - itemSpacing
                        height: itemHeight
                    }
                }
            }
        }
    }
}