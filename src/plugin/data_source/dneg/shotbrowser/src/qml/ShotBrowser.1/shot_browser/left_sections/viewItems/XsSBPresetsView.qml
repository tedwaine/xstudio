// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

Column {
	id: control
	property var treeSequenceModel: null
	property var treeSequenceSelectionModel: null
	property var treeSequenceExpandedModel: null
	property var treeRootIndex: null
	width: parent.width

	Repeater {
		model:DelegateModel {
	        id: tmpmodel
	        property var notifyModel: treeSequenceModel
	        model: notifyModel
	        rootIndex: treeRootIndex

	        delegate: XsSBPresetsViewGroupDelegate{
	        	width: control.width
	            delegateModel: tmpmodel
	            selectionModel: treeSequenceSelectionModel
	            expandedModel: treeSequenceExpandedModel
	        }
	    }
	}


    XsSBPresetEditPopup {
        id: presetEditPopup
    }

    XsPopupMenu {
        id: presetMenu
        property var presetModelIndex: null
        property var filterModelIndex: null
        visible: false
        menu_model_name: "presetMenu"+control

        XsMenuModelItem {
            text: "Duplicate Preset"
            menuItemPosition: 1
            menuPath: ""
            menuModelName: presetMenu.menu_model_name
            onActivated: ShotBrowserEngine.presetsModel.duplicate(presetMenu.presetModelIndex)
        }

        XsMenuModelItem {
            text: "Copy Selected Presets"
            menuItemPosition: 2
            menuPath: ""
            menuModelName: presetMenu.menu_model_name
            onActivated: clipboard.text = JSON.stringify(ShotBrowserEngine.presetsModel.copy(treeSequenceSelectionModel.selectedIndexes))
        }

        XsMenuModelItem {
            text: "Paste Preset"
            menuItemPosition: 3
            menuPath: ""
            menuModelName: presetMenu.menu_model_name
            onActivated: ShotBrowserEngine.presetsModel.paste(
                JSON.parse(clipboard.text),
                presetMenu.presetModelIndex.row+1,
                presetMenu.presetModelIndex.parent
            )
        }
        XsMenuModelItem {
            text: "Remove Preset"
            menuItemPosition: 4
            menuPath: ""
            menuModelName: presetMenu.menu_model_name
            onActivated: {
                let m = presetMenu.presetModelIndex.model
                let sys = m.get(presetMenu.presetModelIndex, "updateRole")
                if(sys != undefined) {
                    m.set(presetMenu.presetModelIndex, true, "hiddenRole")
                } else {
                    ShotBrowserEngine.presetsModel.removeRows(presetMenu.presetModelIndex.row, 1, presetMenu.presetModelIndex.parent)
                }
            }
        }

        XsMenuModelItem {
            menuItemPosition: 5
            menuItemType: "divider"
            menuPath: ""
            menuModelName: presetMenu.menu_model_name
        }

        XsMenuModelItem {
            text: "Move Up"
            menuItemPosition: 6
            menuPath: ""
            menuModelName: presetMenu.menu_model_name
            enabled: presetMenu.filterModelIndex && presetMenu.filterModelIndex.row
            onActivated: {

                // argh this is horridly complex..
                // because we use a view, the previous item in the base model isn't
                // the previous in the view..
                let p = presetMenu.presetModelIndex.parent
                let rpi = presetMenu.filterModelIndex.model.mapToSource(
                	presetMenu.filterModelIndex.model.index(presetMenu.filterModelIndex.row-1,0,presetMenu.filterModelIndex.parent)
                )
                	// delegateModel.modelIndex(index-1)
                ShotBrowserEngine.presetsModel.moveRows(p, presetMenu.presetModelIndex.row, 1, p, rpi.row)
            }
        }


        XsMenuModelItem {
            text: "Move Down"
            menuItemPosition: 7
            menuPath: ""
            menuModelName: presetMenu.menu_model_name
            enabled: presetMenu.filterModelIndex && presetMenu.filterModelIndex ? presetMenu.filterModelIndex.row != presetMenu.filterModelIndex.model.rowCount(presetMenu.filterModelIndex.parent) - 1 : false
            onActivated: {
                let p = presetMenu.presetModelIndex.parent
                let rpi = presetMenu.filterModelIndex.model.mapToSource(
                	presetMenu.filterModelIndex.model.index(presetMenu.filterModelIndex.row+1,0,presetMenu.filterModelIndex.parent)
                )
                ShotBrowserEngine.presetsModel.moveRows(p, presetMenu.presetModelIndex.row, 1, p, rpi.row+1)
            }
        }

        // XsMenuModelItem {
        //     text: "Reset Preset"
        //     menuPath: ""
        //     menuModelName: presetMenu.menu_model_name
        //     enabled: {
        //     	if(presetMenu.presetModelIndex) {
        //     		let v = presetMenu.presetModelIndex.model.get(presetMenu.presetModelIndex,"updateRole")
        //     		return v != undefined ? v : false
        //     	}
        //     	return false
        //     }
        //     onActivated: ShotBrowserEngine.presetsModel.resetPresets([presetMenu.presetModelIndex])
        // }
    }

    XsPopupMenu {
        id: groupMenu

        property var presetModelIndex: null
        property var filterModelIndex: null

        visible: false
        menu_model_name: "groupMenu"+control

        XsMenuModelItem {
            text: "Duplicate Group"
            menuItemPosition: 1
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
            onActivated: groupMenu.presetModelIndex.model.duplicate(groupMenu.presetModelIndex)
        }

        XsMenuModelItem {
            text: "Copy Group"
            menuItemPosition: 2
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
            onActivated: clipboard.text = JSON.stringify(ShotBrowserEngine.presetsModel.copy([groupMenu.presetModelIndex]))
        }
        XsMenuModelItem {
            text: "Paste Group"
            menuItemPosition: 3
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
            onActivated: ShotBrowserEngine.presetsModel.paste(
                JSON.parse(clipboard.text),
                groupMenu.presetModelIndex.row+1,
                groupMenu.presetModelIndex.parent
            )
        }
        XsMenuModelItem {
            text: "Remove Group"
            menuItemPosition: 4
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
            onActivated: {
                let m = groupMenu.presetModelIndex.model
                let sys = m.get(groupMenu.presetModelIndex, "updateRole")
                if(sys != undefined) {
                    m.set(groupMenu.presetModelIndex, true, "hiddenRole")
                } else {
                    ShotBrowserEngine.presetsModel.removeRows(groupMenu.presetModelIndex.row, 1, groupMenu.presetModelIndex.parent)
                }
            }
        }
        XsMenuModelItem {
            menuItemPosition: 5
            menuItemType: "divider"
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
        }

        XsMenuModelItem {
            text: "Paste Preset"
            menuItemPosition: 6
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
            onActivated: {
                ShotBrowserEngine.presetsModel.paste(
                    JSON.parse(clipboard.text),
                    ShotBrowserEngine.presetsModel.rowCount(ShotBrowserEngine.presetsModel.index(1,0,groupMenu.presetModelIndex)),
                    ShotBrowserEngine.presetsModel.index(1,0,groupMenu.presetModelIndex)
                )
            }
        }

        XsMenuModelItem {
            menuItemPosition: 7
            menuItemType: "divider"
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
        }

        XsMenuModelItem {
            text: "Move Up"
            menuItemPosition: 8
            menuPath: ""
            menuModelName: groupMenu.menu_model_name
            enabled: groupMenu.filterModelIndex && groupMenu.filterModelIndex.row
            onActivated: {

                // argh this is horridly complex..
                // because we use a view, the previous item in the base model isn't
                // the previous in the view..
                let p = groupMenu.presetModelIndex.parent
                let rpi = groupMenu.filterModelIndex.model.mapToSource(
                	groupMenu.filterModelIndex.model.index(groupMenu.filterModelIndex.row-1,0,groupMenu.filterModelIndex.parent)
                )
                	// delegateModel.modelIndex(index-1)
                ShotBrowserEngine.presetsModel.moveRows(p, groupMenu.presetModelIndex.row, 1, p, rpi.row)
            }
        }

        XsMenuModelItem {
            text: "Move Down"
            menuItemPosition: 9
            menuPath: ""
            menuModelName: groupMenu.menu_model_name

            enabled: groupMenu.filterModelIndex ? groupMenu.filterModelIndex.row != groupMenu.filterModelIndex.model.rowCount(groupMenu.filterModelIndex.parent) - 1 : false
            onActivated: {
                let p = groupMenu.presetModelIndex.parent
                let rpi = groupMenu.filterModelIndex.model.mapToSource(
                	groupMenu.filterModelIndex.model.index(groupMenu.filterModelIndex.row+1,0,groupMenu.filterModelIndex.parent)
                )
                ShotBrowserEngine.presetsModel.moveRows(p, groupMenu.presetModelIndex.row, 1, p, rpi.row+1)
            }
        }
        // XsMenuModelItem {
        //     text: (groupMenu.presetModelIndex && groupMenu.presetModelIndex.model.get(groupMenu.presetModelIndex,"hiddenRole") ? "Show" : "Hide") + " Group"
        //     menuPath: ""
        //     menuModelName: groupMenu.menu_model_name
        //     onActivated: {
        //     	let m = groupMenu.presetModelIndex.model
        //     	let i = groupMenu.presetModelIndex
        //     	m.set(i, !m.get(i,"hiddenRole"),"hiddenRole")
        //     }
        // }
    }
}
