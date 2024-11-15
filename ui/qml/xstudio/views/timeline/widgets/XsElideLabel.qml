// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Controls


// Qt.ElideLeft
// Qt.ElideMiddle
// Qt.ElideNone
// Qt.ElideRight

Item {
     id: item

     height: label.height

     property string text
     property int elideWidth: width
     property int elide: Qt.ElideRight

     property alias color: label.color
     property alias font: label.font
     property alias horizontalAlignment: label.horizontalAlignment
     property alias verticalAlignment: label.verticalAlignment

     Label {
         id: label
         text: textMetrics.elidedText
         anchors.fill: parent

         TextMetrics {
             id: textMetrics
             text: item.text

             font: label.font

             elide: item.elide
             elideWidth: item.elideWidth
         }
     }
}
