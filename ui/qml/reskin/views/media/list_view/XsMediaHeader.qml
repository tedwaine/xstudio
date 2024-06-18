// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

import xStudioReskin 1.0

import "./widgets"
import "./delegates"

Rectangle{ id: header
    width: parent.width
    height: XsStyleSheet.widgetStdHeight

    color: XsStyleSheet.widgetBgNormalColor

    property bool isSomeColumnResizedByDrag: false
    property alias model: repeater.model

    property alias columns_model: columns_model

    property var columns_model_index: columns_root_model.index(-1, -1)

    XsMediaListColumnsModel {
        id: columns_root_model
    }

    // we need to somehow update these...
    Connections {
        target: columns_root_model
        function onJsonChanged() {
            columns_model_index = columns_root_model.searchRecursive(user_data, "uuid")
        }
    }
    
    Component {
        id: configureDialog
        XsMediaListConfigureDialog {
            onVisibleChanged: {
                if (!visible) {
                    loader.sourceComponent = undefined
                }
            }
        }
    }

    Loader {
        id: loader
    }

    function configure(index) {
        loader.sourceComponent = configureDialog
        loader.item.model_index = index
        loader.item.visible = true
    }    

    DelegateModel {

        id: columns_model
        model: columns_root_model
        rootIndex: columns_model_index

        delegate: XsMediaHeaderColumn{

            Layout.preferredWidth: size ? size : 20
            Layout.minimumHeight: XsStyleSheet.widgetStdHeight 

            titleBarTotalWidth: titleBar.width

            model_index: columns_root_model.index(index, 0 , columns_model_index)

            onHeaderColumnResizing:{
                if(isDragged) {
                    isSomeColumnResizedByDrag = true
                }
                else {
                    isSomeColumnResizedByDrag = false
                }
            }

        }
    }


    Component.onCompleted: {
        if (user_data === undefined) {
            user_data = columns_root_model.new_media_list()
            columns_model_index = columns_root_model.searchRecursive(user_data, "uuid")
        } else {
            columns_model_index = columns_root_model.searchRecursive(user_data, "uuid")
            if (!columns_model_index.valid) {
                user_data = columns_root_model.new_media_list()
                columns_model_index = columns_root_model.searchRecursive(user_data, "uuid")    
            }
        }
    }

    property var metadataPaths: []

    RowLayout{ id: titleBar
        // width: parent.width
        height: parent.height
        spacing: 0

        Repeater{ 
            
            id: repeater
            model: columns_model
        }
    }

}