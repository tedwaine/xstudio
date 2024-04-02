// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
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
