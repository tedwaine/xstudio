// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.models 1.0

Item{ id: presetView

    XsGradientRectangle{
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: (currentCategory == "Tree")? panelPadding : 0
        spacing: panelPadding

        Rectangle{
            Layout.fillWidth: true;
            Layout.preferredHeight: 2;
            color: panelColor
        }

        RowLayout { id: headerDiv
            Layout.fillWidth: true;
            Layout.preferredHeight: btnHeight
            spacing: buttonSpacing

            XsPrimaryButton{ id: addBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/add.svg"
                onClicked: {
                    if(addMenu.visible) addMenu.visible = false
                    else{
                        addMenu.x = x + width
                        addMenu.visible = true
                    }
                }
            }
            // XsPrimaryButton{ id: deleteBtn
            //     Layout.fillWidth: true
            //     Layout.preferredWidth: btnWidth
            //     Layout.maximumWidth: btnWidth
            //     Layout.preferredHeight: parent.height
            //     imgSrc: "qrc:/icons/delete.svg"
            //     onClicked: {
            //         while(presetsSelectionModel.selectedIndexes.length) {
            //             let i = presetsSelectionModel.selectedIndexes[0]
            //             presetsSelectionModel.model.removeRows(i.row, 1, i.parent)
            //         }
            //     }
            // }
            Item{
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height
            }
            XsPrimaryButton{ id: moreBtn
                Layout.preferredWidth: btnWidth
                Layout.preferredHeight: parent.height
                imgSrc: "qrc:/icons/more_vert.svg"
                onClicked:{
                    if(moreMenu.visible) moreMenu.visible = false
                    else{
                        moreMenu.x = x + width
                        moreMenu.visible = true
                    }
                }
            }
        }

        Rectangle{
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            color: panelColor

            Flickable {
                id:flick
                anchors.fill: parent
                clip: true

                contentWidth: width
                contentHeight: tree.height

                ScrollBar.vertical: XsScrollBar{visible: flick.height < flick.contentHeight}

                XsSBPresetsView{
                    id: tree
                    width: flick.width
                    treeSequenceModel: presetsFilterModel
                    treeSequenceSelectionModel: presetsSelectionModel
                    treeSequenceExpandedModel: presetsExpandedModel
                    treeRootIndex: treeSequenceModel ? treeSequenceModel.index(-1,-1) : null
                    onTreeSequenceModelChanged: {
                        if(treeSequenceModel) {
                            treeRootIndex = treeSequenceModel.index(-1,-1)
                        }
                    }
                }
            }
        }

    }


    function addGroupItem(type){

        let m = ShotBrowserEngine.presetsModel
        let i = m.insertGroup(type, -1)

        if(currentCategory == "Tree") {
            m.set(i, "tree", "userdataRole")
            // this shouldn't be required..
            ShotBrowserEngine.presetModels.tree.invalidate()
        }
        else if(currentCategory == "Menus") {
            m.set(i, "menus", "userdataRole")
            ShotBrowserEngine.presetModels.menus.invalidate()
        }
        else if(currentCategory == "Recent") {
            m.set(i, "recent", "userdataRole")
            ShotBrowserEngine.presetModels.recent.invalidate()
        }

    }

    XsPopupMenu {
        id: addMenu
        visible: false
        menu_model_name: "addMenu"+presetView
    }
    XsMenuModelItem {
        property var type: "Versions"
        text: type+" Group"
        menuPath: ""
        menuItemPosition: 1
        menuModelName: addMenu.menu_model_name
        onActivated: {
            addGroupItem(type)
        }
    }
    XsMenuModelItem {
        property var type: "Notes"
        text: type+" Group"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: addMenu.menu_model_name
        onActivated: {
            addGroupItem(type)
        }
    }
    XsMenuModelItem {
        property var type: "Playlists"
        text: type+" Group"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: addMenu.menu_model_name
        onActivated: {
            addGroupItem(type)
        }
    }


    XsPopupMenu {
        id: moreMenu
        visible: false
        menu_model_name: "moreMenu"+presetView
    }

    XsMenuModelItem {
        text: "Undo"
        menuPath: ""
        menuItemPosition: 1
        menuModelName: moreMenu.menu_model_name
        onActivated: ShotBrowserEngine.undo()
    }

    XsMenuModelItem {
        text: "Redo"
        menuPath: ""
        menuItemPosition: 2
        menuModelName: moreMenu.menu_model_name
        onActivated: ShotBrowserEngine.redo()
    }

    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: ""
        menuItemPosition: 3
        menuModelName: moreMenu.menu_model_name
    }

    XsMenuModelItem {
        text: "Reload All System Presets"
        menuPath: ""
        menuItemPosition: 4
        menuModelName: moreMenu.menu_model_name
        onActivated: {
            // set all indexs to visible.
            let hidden = ShotBrowserEngine.presetsModel.searchRecursiveList(
                true, "hiddenRole",
                ShotBrowserEngine.presetsModel.index(-1,-1),
                0,-1
            )
            for(let i=0; i<hidden.length; i++)
                ShotBrowserEngine.presetsModel.set(hidden[i], false, "hiddenRole")

            let changed = ShotBrowserEngine.presetsModel.searchRecursiveList(
                true, "updateRole",
                ShotBrowserEngine.presetsModel.index(-1,-1),
                0,-1
            )
            ShotBrowserEngine.presetsModel.resetPresets(changed)
        }
    }

    // XsMenuModelItem {
    //     text: "Copy Preset"
    //     menuPath: ""
    //     menuItemPosition: 1
    //     menuModelName: moreMenu.menu_model_name
    //     onActivated: clipboard.text = JSON.stringify(ShotBrowserEngine.presetsModel.copy(presetsSelectionModel.selectedIndexes))
    // }
    // XsMenuModelItem {
    //     text: "Paste Preset"
    //     menuPath: ""
    //     menuItemPosition: 2
    //     menuModelName: moreMenu.menu_model_name
    //     onActivated: {
    //         if(presetsSelectionModel.selectedIndexes.length) {
    //             let i = presetsSelectionModel.selectedIndexes[0]
    //             ShotBrowserEngine.presetsModel.paste(
    //                 JSON.parse(clipboard.text),
    //                 i.row+1,
    //                 i.parent
    //             )
    //         }
    //     }
    // }
    // XsMenuModelItem {
    //     text: "Duplicate Preset"
    //     menuPath: ""
    //     menuItemPosition: 2
    //     menuModelName: moreMenu.menu_model_name
    //     onActivated: {
    //         let l = presetsSelectionModel.selectedIndexes
    //         for(let i=0; i< l.length; i++) {
    //             presetsSelectionModel.model.duplicate(l[i])
    //         }
    //     }
    // }
    // XsMenuModelItem {
    //     menuItemType: "divider"
    //     menuPath: ""
    //     menuItemPosition: 5
    //     menuModelName: moreMenu.menu_model_name
    // }
    // XsMenuModelItem {
    //     text: showHiddenPref.value ? "Hide Hidden" : "Show Hidden"
    //     menuPath: ""
    //     menuItemPosition: 6
    //     menuModelName: moreMenu.menu_model_name
    //     onActivated: showHiddenPref.value = !showHiddenPref.value
    // }
    // XsMenuModelItem {
    //     text: {
    //         if(!presetsSelectionModel.selectedIndexes.length)
    //             "Toggle Hidden"
    //         else {
    //             let hidden = presetsSelectionModel.model.get(presetsSelectionModel.selectedIndexes[0], "hiddenRole")
    //             hidden ? "Make Visible" : "Make Hidden"
    //         }
    //     }
    //     menuPath: ""
    //     menuItemPosition: 7
    //     menuModelName: moreMenu.menu_model_name
    //     onActivated: {
    //         let l = presetsSelectionModel.selectedIndexes
    //         if(l.length) {
    //             let hidden = presetsSelectionModel.model.get(l[0], "hiddenRole")
    //             for(let i=0; i< l.length; i++) {
    //                 presetsSelectionModel.model.set(l[i], !hidden, "hiddenRole")
    //             }
    //         }
    //     }

    // }
}