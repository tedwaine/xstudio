// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0
import QtGraphicalEffects 1.0
import QuickFuture 1.0
import QuickPromise 1.0

import xStudioReskin 1.0
import xstudio.qml.helpers 1.0

DelegateChoice {
    roleValue: "Audio Track"

    Component {
		Rectangle {
			id: control

			color: timelineBackground
			property real scaleX: ListView.view.scaleX
			property real scaleY: ListView.view.scaleY
			property real itemHeight: ListView.view.itemHeight
			property real trackHeaderWidth: ListView.view.trackHeaderWidth
			property real cX: ListView.view.cX
		    property real parentWidth: ListView.view.parentWidth
            property var timelineItem: ListView.view.timelineItem
            property string itemFlag: flagColourRole != "" ? flagColourRole : ListView.view.itemFlag
            property var parentLV: ListView.view
		    readonly property bool extraDetail: height > 60
            property var setTrackHeaderWidth: ListView.view.setTrackHeaderWidth

			width: ListView.view.width
			height: itemHeight * scaleY

			property bool isHovered: hoveredItem == control
			property bool isSelected: false
			property bool isConformSource:  modelIndex() == conformSourceIndex
			property var timelineSelection: ListView.view.timelineSelection
            property var hoveredItem: ListView.view.hoveredItem
            property var itemTypeRole: typeRole
            property alias list_view: list_view

			function modelIndex() {
				return control.DelegateModel.model.srcModel.index(
	    			index, 0, control.DelegateModel.model.rootIndex
	    		)
			}

			Connections {
				target: timelineSelection
				function onSelectionChanged(selected, deselected) {
					if(isSelected && helpers.itemSelectionContains(deselected, modelIndex()))
						isSelected = false
					else if(!isSelected && helpers.itemSelectionContains(selected, modelIndex()))
						isSelected = true
				}
			}

		    DelegateChooser {
		        id: chooser
		        role: "typeRole"

		        XsDelegateClip {}
		        XsDelegateGap {}
		    }

		    DelegateModel {
		        id: track_items
		        property var srcModel: theSessionData
		        model: srcModel
		        rootIndex: helpers.makePersistent(control.DelegateModel.model.srcModel.index(
		    		index, 0, control.DelegateModel.model.rootIndex
		    	))
		        delegate: chooser
		    }

	    	XsTrackHeader {
	    		id: track_header
	    		z: 2
				anchors.top: parent.top
				anchors.left: parent.left

				width: trackHeaderWidth
				height: Math.ceil(control.itemHeight * control.scaleY)

				isHovered: control.isHovered
				itemFlag: control.itemFlag
				trackIndex: trackIndexRole
				setTrackHeaderWidth: control.setTrackHeaderWidth
				text: nameRole
				title: "Audio Track"
				isEnabled: enabledRole
				isLocked: lockedRole
				isSelected: control.isSelected
				isConformSource: control.isConformSource
				onEnabledClicked: enabledRole = !enabledRole
				onLockedClicked: lockedRole = !lockedRole
				onConformSourceClicked: conformSourceIndex = helpers.makePersistent(modelIndex())
				onFlagSet: flagItems([modelIndex()], flag == "#00000000" ? "": flag)
	    	}

	    	Flickable {
	    		property bool forceEval: false
				anchors.top: parent.top
				anchors.bottom: parent.bottom
				anchors.left: track_header.right
				anchors.right: parent.right

				interactive: false

				contentWidth: contentItem.childrenRect.width
				contentHeight: contentItem.childrenRect.height
				contentX: (forceEval && !forceEval) || control.cX

				onContentWidthChanged: {
					if(contentX != control.cX) {
						forceEval = !forceEval
					}
				}

		    	Row {
		    	    id:list_view
					opacity: enabledRole ? (lockedRole ? 0.5 : 1.0) : 0.2

			        property real scaleX: control.scaleX
			        property real scaleY: control.scaleY
			        property real itemHeight: control.itemHeight
	    			property var timelineSelection: control.timelineSelection
	                property var timelineItem: control.timelineItem
	                property var hoveredItem: control.hoveredItem
	                property real trackHeaderWidth: control.trackHeaderWidth
					property string itemFlag: control.itemFlag
					property var itemAtIndex: item_repeater.itemAt
		            property var parentLV: control.parentLV

			    	Repeater {
			    		id: item_repeater
						model: track_items
		    		}
		    	}
	    	}
		}
	}
}
