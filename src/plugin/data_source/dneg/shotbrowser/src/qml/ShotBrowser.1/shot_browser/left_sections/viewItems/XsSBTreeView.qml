// SPDX-License-Identifier: Apache-2.0

import QtQuick

import QtQuick.Layouts


import xStudio 1.0
import ShotBrowser 1.0

Column {
	id: control
	width: parent.width

	property var treeSequenceModel: null
	property var treeSequenceSelectionModel: null
	property var treeSequenceExpandedModel: null
	property var treeRootIndex: null

	DelegateModel {
        id: tmpmodel

        property var notifyModel: treeSequenceModel


        model: treeSequenceModel
        rootIndex: treeRootIndex

        delegate: XsSBTreeViewDelegate{
        	width: control.width
            delegateModel: tmpmodel
            selectionModel: treeSequenceSelectionModel
            expandedModel: treeSequenceExpandedModel
        }
    }

	Repeater {
		model: tmpmodel
	}
}
