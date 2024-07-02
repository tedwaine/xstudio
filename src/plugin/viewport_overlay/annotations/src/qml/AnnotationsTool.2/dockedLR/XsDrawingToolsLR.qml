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
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

Item {

    id: drawDialog

    objectName: "XStudioPanel"

    property var preferredWidth: 90
    anchors.fill: parent

    XsGradientRectangle{
        anchors.fill: parent
    }

    property real buttonWidth: 0
    property real buttonHeight: 20
    property real toolPropLoaderHeight: 0
    property real defaultHeight: toolSelector.height + toolProperties.height + toolActionSection.height + framePadding*3
    property bool showButtonHints: false //#TODO: for testing
    property real toolPropertiesWidthThreshold: 200

    property real colSpacing: buttonHeight
    property real itemSpacing: 1 
    property real framePadding: XsStyleSheet.panelPadding/2
    property color toolInactiveTextColor: XsStyleSheet.secondaryTextColor

    property real fontSize: XsStyleSheet.fontSize
    property string fontFamily: XsStyleSheet.fontFamily
    property color textValueColor: "white"

    property int maxDrawSize: 250
    property bool isAnyToolSelected: currentTool !== "None"

    /* This connects to the backend annotations tool object and exposes its
    ui data via model data */
    XsModuleData {
        id: annotations_model_data
        modelDataName: "annotations_tool_settings"
    }

    /* Here we locate particular nodes in the annotations_model_data giving
    convenient access to backend data. Seems crazy but this is the QML way! */
    XsAttributeValue {
        id: draw_pen_size
        attributeTitle: "Draw Pen Size"
        model: annotations_model_data
    }

    XsAttributeValue {
        id: shapes_pen_size
        attributeTitle: "Shapes Pen Size"
        model: annotations_model_data
    }
    XsAttributeValue {
        id: erase_pen_size
        attributeTitle: "Erase Pen Size"
        model: annotations_model_data
    }
    XsAttributeValue {
        id: text_size
        attributeTitle: "Text Size"
        model: annotations_model_data
    }
    XsAttributeValue {
        id: pen_colour
        attributeTitle: "Pen Colour"
        model: annotations_model_data
    }
    XsAttributeValue {
        id: pen_opacity
        attributeTitle: "Pen Opacity"
        model: annotations_model_data
    }
    XsAttributeValue {
        id: active_tool
        attributeTitle: "Active Tool"
        model: annotations_model_data
    }
    XsAttributeValue {
        id: text_background_colour
        attributeTitle: "Text Background Colour"
        model: annotations_model_data
    }
    XsAttributeValue {
        id: text_background_opacity
        attributeTitle: "Text Background Opacity"
        model: annotations_model_data
    }

    // make a local binding to the backend attribute
    property alias currentDrawPenSize: draw_pen_size.value
    property alias currentShapePenSize: shapes_pen_size.value
    property alias currentErasePenSize: erase_pen_size.value
    property alias currentTextSize: text_size.value
    property alias currentToolColour: pen_colour.value
    property alias currentOpacity: pen_opacity.value
    property alias currentTool: active_tool.value
    property alias backgroundColor: text_background_colour.value
    property alias backgroundOpacity: text_background_opacity.value
    
    property var toolSizeAttrName: "Draw Pen Size"

    onCurrentToolChanged: {
        if(currentTool === "Draw")
        {
            currentColorPresetModel = drawColourPresetsModel
            toolSizeAttrName = "Draw Pen Size"
        }
        else if(currentTool === "Erase")
        {
            currentColorPresetModel = eraseColorPresetModel
            toolSizeAttrName = "Erase Pen Size"

        }
        else if(currentTool === "Text")
        {
            currentColorPresetModel = textColourPresetsModel
            toolSizeAttrName = "Text Size"

        }
        else if(currentTool === "Shapes")
        {
            currentColorPresetModel = shapesColourPresetsModel
            toolSizeAttrName = "Shapes Pen Size"

        }
    }


    function setPenSize(penSize) {
        if(currentTool === "Draw")
        {
            currentDrawPenSize = penSize
        }
        else if(currentTool === "Erase")
        {
            currentErasePenSize = penSize
        }
        else if(currentTool === "Shapes")
        {
            currentShapePenSize = penSize
        }
        else if(currentTool === "Text")
        {
            currentTextSize = penSize
        }
    }

    // map the local property for currentToolSize to the backend value ... to modify the tool size, we only change the backend
    // value binding

    property ListModel currentColorPresetModel: drawColourPresetsModel


    XsGradientRectangle{
        anchors.fill: parent
    }
    
    // We wrap all the widgets in a top level Item that can forward keyboard
    // events back to the viewport for consistent
    Item {
        anchors.fill: parent
        focus: true
        // Keys.forwardTo: [sessionWidget] //#TODO - backend

        XsToolSelectorLR {
            id: toolSelector
            x: framePadding/2
            width: parent.width - x*2
            height: toolSet.height + framePadding*2
        }

        Loader { id: toolProperties
            width: toolSelector.width
            height: toolPropLoaderHeight
            x: toolSelector.x
            y: toolSelector.y + toolSelector.height + colSpacing

            sourceComponent: XsToolPropertiesLR{ 
                root: drawDialog
            }            


            ColorDialog { 
                id: colorDialog
                title: "Please pick a colour"
                onAccepted: {
                    currentToolColour = currentColor
                    close()
                }
                onRejected: {
                    close()
                }
                onVisibleChanged: {
                    if (visible) {
                        color = currentToolColour
                    }
                }
            }
            ColorDialog { 
                id: bgColorDialog
                title: "Please pick a BG-Colour"
                onAccepted: {
                    backgroundColor = color
                    close()
                }
                onRejected: {
                    close()
                }
                onVisibleChanged: {
                    if (visible) {
                        color = backgroundColor
                    }
                }

            }


            ListModel{ id: eraseColorPresetModel
                ListElement{
                    preset: "white"
                }
            }
            ListModel{ id: drawColourPresetsModel
                ListElement{
                    preset: "#ff0000" //- "red"
                }
                ListElement{
                    preset: "#ffa000" //- "orange"
                }
                ListElement{
                    preset: "#ffff00" //- "yellow"
                }
                ListElement{
                    preset: "#28dc00" //- "green"
                }
                ListElement{
                    preset: "#0050ff" //- "blue"
                }
                ListElement{
                    preset: "#8c00ff" //- "violet"
                }
                // ListElement{
                //     preset: "#ff64ff" //- "pink"
                // }
                ListElement{
                    preset: "#ffffff" //- "white"
                }
                ListElement{
                    preset: "#000000" //- "black"
                }
            }
            ListModel{ id: textColourPresetsModel
                ListElement{
                    preset: "#ff0000" //- "red"
                }
                ListElement{
                    preset: "#ffa000" //- "orange"
                }
                ListElement{
                    preset: "#ffff00" //- "yellow"
                }
                ListElement{
                    preset: "#28dc00" //- "green"
                }
                ListElement{
                    preset: "#0050ff" //- "blue"
                }
                ListElement{
                    preset: "#8c00ff" //- "violet"
                }
                ListElement{
                    preset: "#ffffff" //- "white"
                }
                ListElement{
                    preset: "#000000" //- "black"
                }
            }
            ListModel{ id: shapesColourPresetsModel
                ListElement{
                    preset: "#ff0000" //- "red"
                }
                ListElement{
                    preset: "#ffa000" //- "orange"
                }
                ListElement{
                    preset: "#ffff00" //- "yellow"
                }
                ListElement{
                    preset: "#28dc00" //- "green"
                }
                ListElement{
                    preset: "#0050ff" //- "blue"
                }
                ListElement{
                    preset: "#8c00ff" //- "violet"
                }
                ListElement{
                    preset: "#ffffff" //- "white"
                }
                ListElement{
                    preset: "#000000" //- "black"
                }
            }

        }

        XsToolActionsLR{ id: toolActionSection
            x: framePadding
            y: !isAnyToolSelected?
                toolProperties.y :
                toolProperties.y + toolProperties.height + colSpacing
            
            // Behavior on y {NumberAnimation{ duration: 250 }}
            
            width: parent.width - framePadding
        }

    }

}