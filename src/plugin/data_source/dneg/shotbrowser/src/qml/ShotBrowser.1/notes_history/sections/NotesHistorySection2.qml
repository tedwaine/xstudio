// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0

Rectangle{
    color: "transparent"

    property bool isHovered: notesDiv.isHovered || toolTipMArea.containsMouse
    // property  alias toolTipMArea: toolTipMArea

    ColumnLayout {
        anchors.fill: parent
        spacing: 1

        NotesHistoryTextRow{ id: subjectDiv
            Layout.fillWidth: true
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight
            text: versionNameRole
        }

        NotesHistoryTextRow{ id: titleDiv
            Layout.fillWidth: true
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight
            text: subjectRole
            textColor: XsStyleSheet.hintColor
        }

        Rectangle{ id: notesDiv
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: XsStyleSheet.widgetBgNormalColor

            property bool isHovered: mArea.containsMouse || notesEdit.hovered || scrollView.hovered

            ScrollView{ id: scrollView
                anchors.fill: parent
                // ScrollBar.vertical.policy: ScrollBar.AsNeeded
                // ScrollBar.vertical.interactive: contentHeight > height
                // wheelEnabled: contentHeight > height
                hoverEnabled: contentHeight > height || contentWidth > width
                // Rectangle{anchors.fill: parent; color: "yellow"; opacity: 0.3; visible: parent.hovered}
                focusPolicy: Qt.ClickFocus
                enabled: false

                TextArea{ id: notesEdit // XsTextEdit{ id: notesEdit
                    // anchors.margins: itemSpacing*2
                    anchors.fill: parent
                    enabled: false
                    readOnly: true
                    clip: true
                    // hoverEnabled: contentHeight > height || contentWidth > width
                    // Rectangle{anchors.fill: parent; color: "red"; opacity: 0.3; visible: parent.hovered}

                    text: contentRole
                    padding: panelPadding
                    // textFormat: parent.lineCount>25 && toolTip.visible? TextEdit.PlainText : TextEdit.AutoText
                    wrapMode: TextEdit.Wrap

                    XsToolTip{
                        id: toolTip
                        text: parent.lineCount>15 ? parent.getFormattedText(0, parent.text.length) : parent.text
                        font.pixelSize: XsStyleSheet.fontSize*0.8
                        visible: toolTipMArea.containsMouse && (scrollView.contentHeight > scrollView.height || scrollView.contentWidth > scrollView.width) //parent.lineCount>7
                        width: metricsDiv.width == 0? 0 : parent.width<200? parent.width+40 : parent.width
                        x: 0
                        timeout: 0
                    }

                }
            }
            MouseArea {
                id: toolTipMArea
                z: 20
                anchors.fill: scrollView
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
            }
            XsImage{
                width: XsStyleSheet.secondaryButtonStdWidth
                height: XsStyleSheet.secondaryButtonStdWidth
                anchors.right: parent.right
                anchors.rightMargin: 2
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 7
                imgOverlayColor: toolTipMArea.containsMouse? palette.highlight : XsStyleSheet.secondaryTextColor //scrollView.ScrollBar.vertical.active?
                source: "qrc:///shotbrowser_icons/arrow_right.svg"
                visible: scrollView.contentWidth > scrollView.width
            }
            XsImage{
                width: XsStyleSheet.secondaryButtonStdWidth
                height: XsStyleSheet.secondaryButtonStdWidth
                anchors.right: parent.right
                anchors.rightMargin: 7
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2
                imgOverlayColor: toolTipMArea.containsMouse? palette.highlight : XsStyleSheet.secondaryTextColor
                source: "qrc:///shotbrowser_icons/arrow_right.svg"
                visible: scrollView.contentHeight > scrollView.height //notesEdit.lineCount > 8
                rotation: 90
            }

        }
    }

}

