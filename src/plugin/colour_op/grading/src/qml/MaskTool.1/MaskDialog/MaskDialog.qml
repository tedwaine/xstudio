// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3 //for ColorDialog
import QtGraphicalEffects 1.15 //for RadialGradient

import xStudioReskin 1.0
import xstudio.qml.bookmarks 1.0
import xstudio.qml.models 1.0

Item {

    id: drawDialog

    property int maxDrawSize: 600

    property real buttonHeight: 20
    property real toolPropLoaderHeight: 0
    property real defaultHeight: toolSelectorFrame.height + toolActionFrame.height + framePadding*3


    property real itemSpacing: framePadding/2
    property real framePadding: 6
    property real framePadding_x2: framePadding*2
    property real frameWidth: 1
    property real frameRadius: 2
    property real frameOpacity: 0.3
    property color frameColor: XsStyleSheet.menuBorderColor


    property color hoverTextColor: palette.text //-whitish //XsStyleSheet.hoverBackground
    property color hoverToolInactiveColor: XsStyleSheet.baseColor //-greyish
    property color toolActiveBgColor: palette.highlight //-orangish
    property color toolActiveTextColor: "white" //palette.highlightedText
    property color toolInactiveBgColor: palette.base //-greyish
    property color toolInactiveTextColor: XsStyleSheet.secondaryTextColor//-greyish

    property real fontSize: XsStyleSheet.menuFontSize/1.1
    property string fontFamily: XsStyleSheet.fontFamily
    property color textButtonColor: toolInactiveTextColor
    property color textValueColor: "white"


    property bool isAnyToolSelected: currentTool !== "None"

    MaskAttrs {
        id: mask_tool_settings
    }

    property int currentToolSize: currentTool === "Erase" ? mask_tool_settings.erase_pen_size : mask_tool_settings.draw_pen_size
    property var currentTool: mask_tool_settings.drawing_tool

    property var toolSizeAttrName: "Draw Pen Size"

    function setPenSize(penSize) {
        if(currentTool === "Draw")
        {
            mask_tool_settings.draw_pen_size = penSize
        }
        else if(currentTool === "Erase")
        {
            mask_tool_settings.erase_pen_size = penSize
        }
    }

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
    }

    // make a read only binding to the "mask_tool_active" backend attribute
    property bool maskToolActive: mask_tool_settings.tool_panel ? mask_tool_settings.tool_panel == "Mask" : false

    // Are we in an active drawing mode?
    property bool drawingActive: maskToolActive && currentTool !== "None"

    // Set the Cursor as required
    property var activeCursor: drawingActive ? Qt.CrossCursor : Qt.ArrowCursor

    /*onActiveCursorChanged: {
        playerWidget.viewport.setRegularCursor(activeCursor)
    }*/

    // map the local property for currentToolSize to the backend value ... to modify the tool size, we only change the backend
    // value binding

    property ListModel currentColorPresetModel: drawColourPresetsModel

    // We wrap all the widgets in a top level Item that can forward keyboard
    // events back to the viewport for consistent
    Item {
        anchors.fill: parent
        focus: true

        Rectangle{
            id: toolSelectorFrame
            width: parent.width - framePadding_x2
            x: framePadding
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.bottom: toolProperties.bottom
            anchors.bottomMargin: -framePadding

            color: "transparent"
            border.width: frameWidth
            border.color: frameColor
            opacity: frameOpacity
            radius: frameRadius

        }

        ToolSelector {
            id: toolSelector
            opacity: 1
            anchors.fill: toolSelectorFrame
        }

        Loader {
            id: toolProperties
            width: toolSelectorFrame.width
            height: toolPropLoaderHeight
            x: toolSelectorFrame.x
            y: buttonHeight*2+framePadding_x2//toolSelectorFrame.toolSelector.y + toolSelectorFrame.toolSelector.height

            sourceComponent:
            Item{

                Row{id: row1
                    x: framePadding //+ itemSpacing/2
                    y: itemSpacing*5 //row1.y + row1.height
                    z: 1
                    width: toolProperties.width - framePadding*2
                    height: (buttonHeight*4) + (spacing*2)
                    spacing: itemSpacing*2

                    Column {
                        z: 2
                        width: parent.width/2-spacing
                        spacing: itemSpacing

                        XsIntegerAttrControl {
                            id: sizeProp
                            visible: isAnyToolSelected && currentTool !== "Shape"
                            width: parent.width-x; height: buttonHeight;
                            text: (currentTool=="Shapes")? "Width" : "Size"
                            enabled: isAnyToolSelected
                            attr_group_model: mask_tool_settings.model
                            attr_title: toolSizeAttrName
                        }

                        XsIntegerAttrControl{
                            id: opacityProp
                            visible: isAnyToolSelected && currentTool != "Erase"
                            width: parent.width-x; height: buttonHeight;
                            text: "Opacity"
                            attr_group_model: mask_tool_settings.model
                            attr_title: "Pen Opacity"
                            enabled: isAnyToolSelected && currentTool != "Erase"
                        }

                        XsIntegerAttrControl{
                            id: softnessProp
                            visible: isAnyToolSelected && currentTool != "Erase"
                            width: parent.width-x; height: buttonHeight;
                            text: "Softness"
                            attr_group_model: mask_tool_settings.model
                            attr_title: "Pen Softness"
                            enabled: isAnyToolSelected && currentTool != "Erase"
                        }

                        XsPrimaryButton{ id: colorProp
                            property bool isPressed: false
                            property bool isMouseHovered: colorMArea.containsMouse
                            enabled: (isAnyToolSelected && currentTool !== "Erase")
                            isActive: isPressed
                            width: parent.width-x; height: buttonHeight;
                            // color: isPressed || isMouseHovered? (enabled? toolActiveBgColor: hoverToolInactiveColor): toolInactiveBgColor;

                            MouseArea{
                                id: colorMArea
                                // enabled: currentTool !== 1
                                hoverEnabled: true
                                anchors.fill: parent
                                onClicked: {
                                        parent.isPressed = false
                                        colorDialog.open()
                                }
                                onPressed: {
                                        parent.isPressed = true
                                }
                                onReleased: {
                                        parent.isPressed = false
                                }
                            }
                            Text{
                                text: "Colour"
                                font.pixelSize: fontSize
                                font.family: fontFamily
                                color: parent.isPressed || parent.isMouseHovered? textValueColor: textButtonColor
                                width: parent.width/2
                                horizontalAlignment: Text.AlignHCenter
                                anchors.right: parent.horizontalCenter
                                anchors.rightMargin: -3
                                topPadding: framePadding/1.2
                            }
                            Rectangle{ id: colorPreviewDuplicate
                                opacity: (!isAnyToolSelected || currentTool === "Erase")? (parent.enabled?1:0.5): 0
                                height: parent.height/1.4;
                                color: currentTool === "Erase" ? "white" : mask_tool_settings.pen_colour
                                border.width: frameWidth
                                border.color: parent.enabled? (mask_tool_settings.pen_colour=="white" || mask_tool_settings.pen_colour=="#ffffff")? "black": "white" : Qt.darker("white",1.5)
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.horizontalCenter
                                anchors.leftMargin: parent.width/7
                                anchors.right: parent.right
                                anchors.rightMargin: parent.width/10
                            }
                            Rectangle{ id: colorPreview
                                visible: (isAnyToolSelected && currentTool !== "Erase")
                                x: colorPreviewDuplicate.x
                                y: colorPreviewDuplicate.y
                                width: colorPreviewDuplicate.width
                                onWidthChanged: {
                                    x= colorPreviewDuplicate.x
                                    y= colorPreviewDuplicate.y
                                }
                                height: colorPreviewDuplicate.height
                                color: currentTool === "Erase" ? "white" : mask_tool_settings.pen_colour;
                                border.width: frameWidth;
                                border.color: (color=="white" || color=="#ffffff")? "black": "white"

                                scale: dragArea.drag.active? 0.6: 1
                                Behavior on scale {NumberAnimation{ duration: 250 }}

                                Drag.active: dragArea.drag.active
                                Drag.hotSpot.x: colorPreview.width/2
                                Drag.hotSpot.y: colorPreview.height/2
                                MouseArea{
                                    id: dragArea
                                    anchors.fill: parent
                                    drag.target: parent

                                    drag.minimumX: -framePadding
                                    drag.maximumX: toolSelectorFrame.width - framePadding*5
                                    drag.minimumY: buttonHeight
                                    drag.maximumY: buttonHeight*2.5

                                    onReleased: {
                                        colorProp.isPressed = false
                                        parent.Drag.drop()
                                        parent.x = colorPreviewDuplicate.x
                                        parent.y = colorPreviewDuplicate.y
                                    }
                                    onClicked: {
                                        colorProp.isPressed = false
                                        colorDialog.open()
                                    }
                                    onPressed: {
                                        colorProp.isPressed = true
                                    }
                                }
                            }
                        }
                    }

                    XsPrimaryButton {
                        width: softnessProp.width; height: softnessProp.height
                        visible: isAnyToolSelected && currentTool === "Shape"
                        isActive: mask_tool_settings.shape_invert
                        text: "Invert"
                        onClicked: {
                            mask_tool_settings.shape_invert = !mask_tool_settings.shape_invert
                        }
                    }

                    Rectangle { id: toolPreview
                        width: parent.width/2
                        height: parent.height
                        color: "#595959" //"transparent"
                        border.color: frameColor
                        border.width: frameWidth
                        // clip: true
                        visible: (isAnyToolSelected && currentTool !== "Shape")

                        Grid {id: checkerBg;
                            property real tileSize: framePadding
                            anchors.fill: parent;
                            anchors.centerIn: parent
                            anchors.margins: tileSize/2;
                            clip: true;
                            rows: Math.floor(height/tileSize);
                            columns: Math.floor(width/tileSize);
                            Repeater {
                                model: checkerBg.columns*checkerBg.rows
                                Rectangle {
                                    property int oddRow: Math.floor(index / checkerBg.columns)%2
                                    property int oddColumn: (index % checkerBg.columns)%2
                                    width: checkerBg.tileSize; height: checkerBg.tileSize
                                    color: (oddRow == 1 ^ oddColumn == 1) ? "#949494": "#595959"
                                }
                            }
                        }

                        Rectangle{

                            id: clippedPreview
                            anchors.fill: parent
                            color: "transparent"
                            clip: true

                            Rectangle {id: drawPreview
                                visible: currentTool === "Draw"
                                anchors.centerIn: parent
                                property real sizeScaleFactor: (parent.height)/maxDrawSize
                                width: currentToolSize *sizeScaleFactor
                                height: width
                                radius: width/2
                                color: mask_tool_settings.pen_colour
                                opacity: mask_tool_settings.pen_opacity/100

                                RadialGradient {
                                    visible: false
                                    anchors.fill: parent
                                    source: parent
                                    gradient:
                                    Gradient {
                                        GradientStop {
                                            position: 0.1; color: mask_tool_settings.pen_colour
                                        }
                                        GradientStop {
                                            position: 1.0; color: "black"
                                        }
                                    }
                                }

                            }

                            Rectangle { id: erasePreview
                                visible: currentTool === "Erase"
                                anchors.centerIn: parent
                                property real sizeScaleFactor: (parent.height)/maxDrawSize
                                width: currentToolSize * sizeScaleFactor
                                height: width
                                radius: width/2
                                color: "white"
                                opacity: 1
                            }

                        }
                    }
                }


                Rectangle{ id: row2
                    y: row1.y + row1.height + presetColours.spacing
                    width: toolProperties.width
                    height: buttonHeight *1.5
                    visible: (isAnyToolSelected && currentTool !== "Erase" && currentTool !== "Shape")
                    color: "transparent"

                    ListView{ id: presetColours
                        x: frameWidth +spacing*2
                        width: parent.width - frameWidth*2 - spacing*2
                        height: parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: (itemSpacing!==0)?itemSpacing/2: 0
                        clip: true
                        interactive: false
                        orientation: ListView.Horizontal

                        model: currentColorPresetModel
                        delegate:
                        Item{
                            property bool isMouseHovered: presetMArea.containsMouse
                            width: presetColours.width/9-presetColours.spacing;
                            height: presetColours.height
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width
                                height: width
                                radius: width/2
                                color: preset
                                border.width: 1
                                border.color: parent.isMouseHovered? toolActiveBgColor: (mask_tool_settings.pen_colour === preset)? toolActiveTextColor: "black"

                                MouseArea{
                                    id: presetMArea
                                    property color temp_color
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {

                                        temp_color = currentColorPresetModel.get(index).preset;
                                        mask_tool_settings.pen_colour = temp_color

                                    }
                                }

                                DropArea {
                                    anchors.fill: parent
                                    Image {
                                        visible: parent.containsDrag
                                        anchors.fill: parent
                                        source: "qrc:///feather_icons/plus-circle.svg"
                                        layer {
                                            enabled: (preset=="black" || preset=="#000000")
                                            effect:
                                            ColorOverlay {
                                                color: "white"
                                            }
                                        }
                                    }
                                    onDropped: {
                                        currentColorPresetModel.setProperty(index, "preset", mask_tool_settings.pen_colour.toString())
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle{ id: row2bis
                    x: toolSelectorFrame.x
                    y: row1.y + row1.height - presetColours.height / 2
                    width: toolProperties.width
                    height: buttonHeight *1.5
                    visible: (isAnyToolSelected && currentTool === "Shape")
                    color: "transparent"

                    Column {
                        spacing: 2

                        Row {
                            spacing: 2

                            XsPrimaryButton {
                                text: "Quad"
                                width: toolActionUndoRedo.width/modelUndoRedo.count - toolActionUndoRedo.spacing
                                height: buttonHeight
                                onClicked: {
                                    mask_tool_settings.drawing_action = "Add quad"
                                }
                            }
                            XsPrimaryButton {
                                text: "Ellipse"
                                width: toolActionUndoRedo.width/modelUndoRedo.count - toolActionUndoRedo.spacing
                                height: buttonHeight
                                onClicked: {
                                    mask_tool_settings.drawing_action = "Add ellipse"
                                }
                            }
                        }

                        Row {
                            spacing: 2

                            XsPrimaryButton {
                                text: "Polygon"
                                width: toolActionUndoRedo.width/modelUndoRedo.count - toolActionUndoRedo.spacing
                                height: buttonHeight
                                isActive: mask_tool_settings.polygon_init
                                onClicked: {
                                    mask_tool_settings.polygon_init = !mask_tool_settings.polygon_init
                                }
                            }
                            XsPrimaryButton {
                                text: "Remove"
                                width: toolActionUndoRedo.width/modelUndoRedo.count - toolActionUndoRedo.spacing
                                height: buttonHeight
                                onClicked: {
                                    mask_tool_settings.drawing_action = "Remove shape"
                                }
                            }
                        }
                    }
                }

                Component.onCompleted: {
                    toolPropLoaderHeight = row2.y + row2.height
                }
            }


            ColorDialog { id: colorDialog
                title: "Please pick a color"
                color: mask_tool_settings.pen_colour
                onAccepted: {
                    mask_tool_settings.pen_colour = currentColor
                    close()
                }
                onRejected: {
                    close()
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
                ListElement{
                    preset: "#ff64ff" //- "pink"
                }
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
                    preset: "#ff64ff" //- "pink"
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
                    preset: "#ff64ff" //- "pink"
                }
                ListElement{
                    preset: "#ffffff" //- "white"
                }
                ListElement{
                    preset: "#000000" //- "black"
                }
            }
        }

        Rectangle{ id: toolActionFrame
            x: framePadding
            anchors.top: toolSelectorFrame.bottom
            anchors.topMargin: framePadding

            width: parent.width - framePadding_x2
            height: toolSelectorFrame.height/2.4

            color: "transparent"
            opacity: frameOpacity
            border.width: frameWidth
            border.color: frameColor
            radius: frameRadius
        }
        Item{ id: toolActionSection
            x: toolActionFrame.x
            width: toolActionFrame.width

            ListView{ id: toolActionUndoRedo

                width: parent.width - framePadding_x2
                height: buttonHeight
                x: framePadding + spacing/2
                y: toolActionFrame.y + framePadding + spacing/2

                spacing: itemSpacing
                clip: true
                interactive: false
                orientation: ListView.Horizontal

                model:
                ListModel{
                    id: modelUndoRedo
                    ListElement{
                        action: "Undo"
                    }
                    ListElement{
                        action: "Redo"
                    }
                }
                delegate:
                XsPrimaryButton{
                    text: model.action
                    width: toolActionUndoRedo.width/modelUndoRedo.count - toolActionUndoRedo.spacing
                    height: buttonHeight
                    onClicked: {
                        mask_tool_settings.drawing_action = text
                    }
                }
            }

            ListView{ id: toolActionCopyPasteClear

                width: parent.width - framePadding_x2
                height: buttonHeight
                x: framePadding + spacing/2
                y: toolActionUndoRedo.y + toolActionUndoRedo.height + spacing

                spacing: itemSpacing
                clip: true
                interactive: false
                orientation: ListView.Horizontal

                model:
                ListModel{
                    id: modelCopyPasteClear
                    ListElement{
                        action: "Copy"
                    }
                    ListElement{
                        action: "Paste"
                    }
                    ListElement{
                        action: "Clear"
                    }
                }
                delegate:
                XsPrimaryButton{
                    text: model.action
                    width: toolActionCopyPasteClear.width/modelCopyPasteClear.count - toolActionCopyPasteClear.spacing
                    height: buttonHeight
                    enabled: text == "Clear"
                    onClicked: {
                        mask_tool_settings.drawing_action = text
                    }

                }
            }

            ListView{ id: toolActionDisplayMode

                width: parent.width - framePadding_x2
                height: buttonHeight
                x: framePadding + spacing/2
                y: toolActionCopyPasteClear.y + toolActionCopyPasteClear.height + spacing

                spacing: itemSpacing
                clip: true
                interactive: false
                orientation: ListView.Horizontal

                model:
                ListModel{
                    id: modelDisplayMode
                    ListElement{
                        action: "Mask"
                        //tooltip: "Show mask being draw"
                    }
                    ListElement{
                        action: "Grade"
                        //tooltip: "Show masked grade result"
                    }
                }
                delegate:
                XsPrimaryButton{
                    isActive: mask_tool_settings.display_mode == text
                    text: model.action
                    //tooltip: model.tooltip
                    width: toolActionDisplayMode.width/modelDisplayMode.count - toolActionDisplayMode.spacing
                    height: buttonHeight
                    onClicked: {
                        mask_tool_settings.display_mode = text
                    }
                }
            }

        }
    }

}