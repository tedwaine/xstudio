import QtQuick
import QtQuick.Layouts
import xStudio 1.0

Item {

    id: view
    property var possibleSources: ["No Media"]
    property var parent_menu
    onVisibleChanged: {
        if (visible) {
            if (!filteredmediaModelData.selectedSourceIndexes.length) {
                possibleSources = []
                return
            }
            // use first item in media list selection
            var l = filteredmediaModelData.selectedSourceIndexes[0]
            // loop over sources
            let v = []
            let rc = l.model.rowCount(l)
            for (var i = 0; i < rc; i++) {
                let idx = l.model.index(i, 0, l)
                let name = l.model.getData(idx, "nameRole")
                // do something with each source if needed
                v.push(name)
            }
            // get the indexes to the media sources for the media item
            possibleSources = v
        }
    }

    property var minWidth: 50
    width: parent.width
    height: layout.height

    function setMinWidth(mw) {
        if (mw > minWidth) {
            minWidth = mw
        }
    }
    ColumnLayout {
        id: layout
        width: minWidth
        Repeater {
            model: possibleSources
            XsMenuItemBasic {
                Layout.fillWidth: true
                menu_item_text: possibleSources[index]
                onClicked: {
                    filteredmediaModelData.selectedSourceIndexes.forEach(
                        (media_idx) => {
                            // media_idx is a media item in the model
                            // loop over rows at media_idx (media sources)
                            let rc = media_idx.model.rowCount(media_idx)
                            for (var i = 0; i< rc; i++) {
                                let media_source_idx = media_idx.model.index(i, 0, media_idx)
                                let name = media_source_idx.model.getData(media_source_idx, "nameRole")
                                // do something with each source if needed
                                if (name == menu_item_text) {
                                    media_idx.model.setRoleData(
                                        media_idx,
                                        "imageActorUuidRole",
                                        media_source_idx.model.getData(
                                            media_source_idx,
                                            "uuidRole"
                                        )
                                    )
                                }
                            }
                        }
                    )
                    if (parent_menu) parent_menu.closeAll()
                }
                onMinWidthChanged: {
                    view.setMinWidth(minWidth)
                }
            }
        }
    }
}