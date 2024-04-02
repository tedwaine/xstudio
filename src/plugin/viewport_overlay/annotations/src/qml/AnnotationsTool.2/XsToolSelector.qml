// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3
import QtGraphicalEffects 1.15

import xStudioReskin 1.0
import xstudio.qml.module 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

Item{
    id: toolSetBg

    property int rowItemCount: 5
    property real itemWidth: (width-framePadding*2)/rowItemCount
    onItemWidthChanged: {
        buttonWidth = itemWidth
    }
    property real itemHeight: XsStyleSheet.primaryButtonStdHeight

    /* This connects to the backend annotations tool object and exposes its
    ui data via model data */
    XsModuleData {
        id: annotations_tool_types
        modelDataName: "annotations_tool_types_0"
    }

    XsAttributeValue {
        id: tool_types_value
        attributeTitle: "Active Tool"
        model: annotations_tool_types
    }
    XsAttributeValue {
        id: tool_types_choices
        role: "combo_box_options"
        attributeTitle: "Active Tool"
        model: annotations_tool_types
    }

    property alias current_tool: tool_types_value.value

    // adding 'Laser' here as it's not actually part of combo_box_options
    // because currently laser is a separate toggle attribute and we can't
    // change it without breaking the 'old' UI.
    property var tool_choices: {
        if (tool_types_choices.value) {
            var r = tool_types_choices.value
            r.push("Laser")
            return r;
        }
        return []
    }

    // Un-comment this when Laser is implemented in combo_box_options
    // property alias tool_choices: tool_types_choices.value

    property var toolImages: [
        "qrc:///anno_icons/draw_brush.svg",
        "qrc:///anno_icons/draw_shapes.svg",
        "qrc:///anno_icons/draw_text.svg",
        "qrc:///anno_icons/draw_eraser.svg",
        "qrc:///anno_icons/draw_laser.svg"
    ]

    GridView{

        id: toolSet

        x: framePadding
        y: framePadding

        width: itemWidth*rowItemCount
        height: itemHeight

        cellWidth: itemWidth
        cellHeight: itemHeight

        interactive: false
        flow: GridView.FlowLeftToRight

        // read only convenience binding to backend.
        currentIndex: tool_choices ? tool_choices.indexOf(current_tool) : undefined

        model: tool_choices // this is 'role data' from the backend attr

        delegate: toolSetDelegate

        Component{

            id: toolSetDelegate

            Rectangle{
                width: toolSet.cellWidth
                height: toolSet.cellHeight
                color: "transparent"
                property bool isEnabled: index!=5 && index!=6

                XsPrimaryButton{ id: toolBtn

                    x: index>=rowItemCount? width/2:0
                    width: index==(rowItemCount-1)? parent.width : parent.width - itemSpacing
                    height: parent.height - itemSpacing
            
                    clip: true
                    isToolTipEnabled: showButtonHints

                    imageDiv.opacity: isEnabled ? 1.0 : 0.33
                    isActive: current_tool===text
                    anchors.top: parent.top

                    text: tool_choices[index]
                    imgSrc: toolImages[index]
                    enabled: index < 4 // exlclude "Laser" mode for now, see note above
                    visible: index < 5 // exlclude "Laser" mode for now, see note above

                    onClicked: {
                        if (!isEnabled) return;
                        if(isActive)
                        {
                            //Disables tool by setting the 'value' of the 'active tool'
                            // attribute in the plugin backend to 'None'
                            current_tool = "None"
                        }
                        else
                        {
                            current_tool = text
                        }
                    }
                    onPressAndHold:{ //#TODO: for testing
                        showButtonHints = !showButtonHints
                    }

                }

            }
        }

    }

}
