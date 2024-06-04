import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import QtQml.Models 2.14

import xStudioReskin 1.0
import xstudio.qml.models 1.0

XsListView { 
        
    id: row
    orientation: ListView.Horizontal
    property string placement: "left"
    property var dockedWidgetsModel
    interactive: false

    // when dockedWidgetsModel array changes, qml can only say the whole thing
    // has changed (even when a single element has changed). So if our delegate
    // model was driven directly by dockedWidgetsModel all the widgets would
    // instantaneously be re-created or destroyed in response to a change so
    // we couldn't do fancy show/hide animation.
    // So for coherent tracking of which widgets are visible or not (to support
    // the animation) we build a ListModel from dockedWidgetsModel
    ListModel{ 
        id: docked_widgets
    }

    onDockedWidgetsModelChanged: {

        if (dockedWidgetsModel == undefined) return

        // dockedWidgetsModel is provided and managed by the parent widget
        // 'dockedWidgetsModel' here is an array - each element in the array should be
        // an array of length 3: [widget_name, visibility, placement]
        // where 'placement' will be 'left', 'right', 'top' or 'bottom' and
        // visibility is true or false
        var visible_widgets = []
        for (var i = 0; i < dockedWidgetsModel.length; ++i) {
            if (dockedWidgetsModel[i][2] == placement) {
                if (dockedWidgetsModel[i][1]) {
                    visible_widgets.push(dockedWidgetsModel[i][0])
                }
            }
        }

        // hide/show any existing widgets that aren't in visible_widgets
        for (var j = 0; j < docked_widgets.count; ++j) {
            if (visible_widgets.indexOf(docked_widgets.get(j).widget_name) == -1) {
                docked_widgets.get(j).showing = false
            } else {
                docked_widgets.get(j).showing = true
            }
        }

        // add widgets that are new
        for (var i = 0; i < visible_widgets.length; ++i) {
            var widget_name = visible_widgets[i]
            var found = false
            for (var j = 0; j < docked_widgets.count; ++j) {
                if (docked_widgets.get(j).widget_name == widget_name) {
                    found = true
                    break
                }
            }
            if (!found) {
                docked_widgets.append({"widget_name": widget_name, "showing": true})
            }
        }
    }

    DelegateModel {
        id: delegate_model
        model: docked_widgets
        delegate: Item {
            id: container
            clip: true
            height: row.height
            property var widgetName: widget_name
            property var dynamic_widget
            onWidgetNameChanged: {
                var idx = dockables.searchRecursive(widgetName, "title")
                var source = dockables.get(idx, "qml_code")
                if (source != undefined && source != "") {
                    dynamic_widget = Qt.createQmlObject(source, container)
                }
            }
            states: [
                State {
                    name: "showing"
                    when: showing
                    PropertyChanges { target: container; implicitWidth: dynamic_widget.preferredWidth}
                },
                State {
                    name: "hiding"
                    when: !showing
                    PropertyChanges { target: container; implicitWidth: 0}
                }
            ]

            transitions: Transition {
                NumberAnimation { properties: "implicitWidth"; duration: 150 }
            }

            XsSecondaryButton {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.margins: 8
                width: 16
                height: 16
                z: 100
                imgSrc: "qrc:/icons/chevron_right.svg"
                imageDiv.rotation: placement == "left" ? 0 : 180
                onClicked: {
                    if (placement == "left") {
                        move_dockable_widget(widgetName, "right")
                    } else {
                        move_dockable_widget(widgetName, "left")
                    }
                }
            }

        }
    }
    
    model: delegate_model

    XsModuleData {
        id: dockables
        modelDataName: "dockable viewport toolboxes"
    }

}