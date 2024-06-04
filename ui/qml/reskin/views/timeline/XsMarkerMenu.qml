import xstudio.qml.models 1.0
import xStudioReskin 1.0
import QtQml.Models 2.14

XsPopupMenu {

    id: markerMenu
    visible: false
    menu_model_name: "marker_menu_"+markerMenu

    property var panelContext: helpers.contextPanel(timelineMenu)
    property var theTimeline: panelContext.theTimeline
    property var markerIndex: null


    XsMenuModelItem {
        text: qsTr("Remove Marker")
        menuPath: ""
        menuItemPosition: 1
        menuModelName: markerMenu.menu_model_name
        onActivated: markerIndex.model.removeRows(markerIndex.row, 1, markerIndex.parent)
    }
}