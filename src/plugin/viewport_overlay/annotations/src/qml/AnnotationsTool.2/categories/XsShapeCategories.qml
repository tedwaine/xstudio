// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import QtQml.Models 2.14

import xStudioReskin 1.0
import xstudio.qml.module 1.0

Item{ 
                    
    id: shapeCategories

    property alias shapesModel: shapesModel

    XsModuleAttributes {
        id: anno_tool_backend_settings
        attributesGroupNames: "annotations_tool_settings_0"
    }

    // make a local binding to the backend attribute
    property int shapeTool: anno_tool_backend_settings.shape_tool ? anno_tool_backend_settings.shape_tool : 0.0

    onShapeToolChanged: {
        shapesList.currentIndex = shapeTool
    }

    ListView{ id: shapesList

        width: parent.width
        height: buttonHeight
        x: framePadding + spacing/2

        spacing: itemSpacing
        interactive: false
        orientation: ListView.Horizontal

        model: shapesModel

        onCurrentIndexChanged: {
            if (anno_tool_backend_settings.shape_tool != undefined) {
                anno_tool_backend_settings.shape_tool = currentIndex
            }
        }

        delegate: XsPrimaryButton{ 
            id: shapeBtn
            text: shapeHint
            isToolTipEnabled: showButtonHints

            imgSrc: shapeImage
            imageDiv.sourceSize.height: 16
            imageDiv.sourceSize.width: 16

            width: index==(shapesModel.count-1)?
                shapesList.width/shapesModel.count :
                shapesList.width/shapesModel.count - shapesList.spacing
            height: shapesList.height

            isActive: shapesList.currentIndex===index
            onClicked: {
                shapesList.currentIndex = index
            }
        }
    }

    ListModel{ id: shapesModel //#TODO: to update SVG1, SVG2

        ListElement{
            shapeHint: "Square"
            shapeImage: "qrc:///anno_icons/draw_shape_square.svg" //"qrc:///feather_icons/square.svg"
            shapeSVG1: "data: image/svg+xml;utf8, <svg viewBox=\"0 0 48 48\" width=\"100%\" height=\"100%\" stroke=\"white\" stroke-width=\""
            shapeSVG2: "\" fill=\"none\" stroke-linecap=\"round\" stroke-linejoin=\"round\" ><rect x=\"6\" y=\"6\" width=\"36\" height=\"36\" rx=\"4\" ry=\"4\"></rect></svg>"
        }
        ListElement{
            shapeHint: "Circle"
            shapeImage: "qrc:///anno_icons/draw_shape_circle.svg"
            shapeSVG1: "data: image/svg+xml;utf8, <svg viewBox=\"0 0 48 48\" width=\"100%\" height=\"100%\" stroke=\"white\" stroke-width=\""
            shapeSVG2: "\" fill=\"none\" stroke-linecap=\"round\" stroke-linejoin=\"round\" ><circle cx=\"24\" cy=\"24\" r=\"20\"></circle></svg>"
        }
        ListElement{
            shapeHint: "Arrow"
            shapeImage: "qrc:///anno_icons/draw_shape_arrow.svg"
            shapeSVG1: "data: image/svg+xml;utf8, <svg viewBox=\"0 0 48 48\" width=\"100%\" height=\"100%\" stroke=\"white\" stroke-width=\""
            shapeSVG2: "\" fill=\"none\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><line x1=\"10\" y1=\"24\" x2=\"38\" y2=\"24\"></line><polyline points=\"24 10 38 24 24 38\"></polyline></svg>"
        }
        ListElement{
            shapeHint: "Line"
            shapeImage: "qrc:///anno_icons/draw_shape_line.svg"
            shapeSVG1: "data: image/svg+xml;utf8, <svg viewBox=\"0 0 48 48\" width=\"100%\" height=\"100%\" stroke=\"white\" stroke-width=\""
            shapeSVG2: "\" fill=\"none\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><line x1=\"10\" y1=\"24\" x2=\"38\" y2=\"24\"></line></svg>"
        }

    }

    function shapePreviewSVGSource(tool_size) {
        return shapesModel.get(shapesList.currentIndex).shapeSVG1 + (tool_size*7/100) + shapesModel.get(shapesList.currentIndex).shapeSVG2
    }

}
