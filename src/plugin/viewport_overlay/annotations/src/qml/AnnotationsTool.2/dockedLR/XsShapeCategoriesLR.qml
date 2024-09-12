// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import xstudio.qml.bookmarks 1.0
import QtQml.Models 2.14

import xStudio 1.0
import xstudio.qml.models 1.0

Item{

    id: shapeCategories

    property alias shapesModel: shapesModel

    XsAttributeValue {
        id: __shape_tool
        attributeTitle: "Shape Tool"
        model: annotations_model_data
    }
    property alias shape_tool: __shape_tool.value

    // make a local binding to the backend attribute
    property int shapeTool: shape_tool

    onShapeToolChanged: {
        shapesList.currentIndex = shapeTool
    }

    GridView{ id: shapesList

        width: parent.width
        height: buttonHeight*2
        x: framePadding + itemSpacing/2

        cellWidth: width/2
        cellHeight: buttonHeight
        // spacing: itemSpacing
        interactive: false
        flow: GridView.FlowLeftToRight

        model: shapesModel

        onCurrentIndexChanged: {
            if (shape_tool != undefined) {
                shape_tool = currentIndex
            }
        }

        delegate: XsPrimaryButton{
            id: shapeBtn
            text: shapeHint
            isToolTipEnabled: showButtonHints

            imgSrc: shapeImage
            imageDiv.height: 16
            imageDiv.width: 16

            width: index%2==0? shapesList.cellWidth - itemSpacing : shapesList.cellWidth
            height: shapesList.cellHeight - itemSpacing

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
