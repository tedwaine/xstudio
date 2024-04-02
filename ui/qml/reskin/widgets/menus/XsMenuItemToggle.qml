import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.3

import xStudioReskin 1.0
import xstudio.qml.models 1.0

Item {

    id: widget            
    height: XsStyleSheet.menuHeight

    // Note .. menu item width needs to be set by the widest menu item in the
    // same menu. This creates a circular dependency .. the menu item width
    // depends on the widest item. If it is the widest item its width 
    // depends on itself. There must be a better QML solution for this
    property real minWidth: final_spacer.width + indentSpacer.width + iconDiv.width + hotkey_metrics.width + label_metrics.width + margin*4    

    property real leftIconSize: indentSpacer.width + iconDiv.width

    property bool isChecked: isRadioButton? radioSelectedChoice==label : is_checked
    property bool isRadioButton: false
    property var radioSelectedChoice: ""
    property var label: name ? name : ""

    property var margin: 4 // don't need this ?
    property var sub_menu
    property var menu_model
    property var menu_model_index
    property var parent_menu
    property alias icon: iconDiv.source

    property bool isHovered: menuMouseArea.containsMouse
    property bool isActive: menuMouseArea.pressed
 
    property color hotKeyColor: palette.highlight
    property real borderWidth: XsStyleSheet.widgetBorderWidth

    opacity: enabled ? 1 : 0.5

    function hideSubMenus() {}

    signal clicked()

    MouseArea{
        id: menuMouseArea
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        onClicked: {
            widget.clicked()
        }
    }

    Rectangle { 
        id: bgHighlightDiv
        implicitWidth: parent.width
        implicitHeight: parent.height
        anchors.verticalCenter: parent.verticalCenter
        border.color: palette.highlight
        border.width: borderWidth
        color: isActive ? palette.highlight : "transparent"
        visible: widget.isHovered

    }

    RowLayout {

        anchors.fill: parent
        anchors.margins: margin
        spacing: 0

        XsImage { 
        
            id: iconDiv
            source: isRadioButton? 
                isChecked ? "qrc:/icons/radio_button_checked.svg" : "qrc:/icons/radio_button_unchecked.svg" :
                isChecked ? "qrc:/icons/check_box_checked.svg" : "qrc:/icons/check_box_unchecked.svg"
            width: XsStyleSheet.menuCheckboxSize
            height: XsStyleSheet.menuCheckboxSize
            sourceSize.height: height
            sourceSize.width: width
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            layer {
                enabled: true
                effect: ColorOverlay { color: hotKeyColor }
            }            
    
        }
    
        Item {
            id: indentSpacer
            width: 3
        }

        Text { 
            
            id: labelDiv
            text: label ? label : "Unknown" //+ (sub_menu && !is_in_bar ? "   >>" : "")
            font.pixelSize: XsStyleSheet.fontSize
            font.family: XsStyleSheet.fontFamily
            color: palette.text 
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

        }

        TextMetrics {
            id:     label_metrics
            font:   labelDiv.font
            text:   labelDiv.text
        }

        Item {
            Layout.fillWidth: true
        }

        Text { 

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            id: hotKeyDiv
            text: hotkey_sequence ? hotkey_sequence : ""
            font: labelDiv.font
            color: hotKeyColor 
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }

        TextMetrics {
            id:     hotkey_metrics
            font:   hotKeyDiv.font
            text:   hotKeyDiv.text
        }

        Item {
            id: final_spacer
            width : 4
        }
    }
}