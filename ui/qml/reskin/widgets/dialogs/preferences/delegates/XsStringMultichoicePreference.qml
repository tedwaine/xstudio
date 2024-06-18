// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Qt.labs.qmlmodels 1.0

import xstudio.qml.models 1.0
import xStudioReskin 1.0

import "../widgets"

RowLayout {

    width: parent.width
    height: XsStyleSheet.widgetStdHeight

    function spangle(oof) {
        console.log("Setting ", valueRole, oof)
        if (valueRole != oof) {
            valueRole = oof
        }
        
    }

    XsLabel {
        Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        Layout.preferredWidth: parent.width/2 //prefsLabelWidth
        Layout.maximumWidth: parent.width/2 

        text: displayNameRole
        horizontalAlignment: Text.AlignRight
    }

    XsComboBox { 
        
        id: combo_box
        Layout.alignment: Qt.AlignVCenter|Qt.AlignLeft
        Layout.preferredWidth: prefsLabelWidth
        Layout.minimumWidth: prefsLabelWidth/2
        Layout.fillHeight: true
        model: optionsRole

        onCurrentIndexChanged: {
            if (optionsRole != undefined) {
                spangle(optionsRole[currentIndex])
            }
        }

        property var value__: valueRole
        onValue__Changed: {
            if (optionsRole != undefined && optionsRole.indexOf(value__) != -1) {
                currentIndex = optionsRole.indexOf(value__)
            }
        }
    }

    XsPreferenceInfoButton {
    }

}
