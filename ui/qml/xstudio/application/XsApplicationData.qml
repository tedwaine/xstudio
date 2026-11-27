import QtQuick
import QuickFuture 1.0

import xstudio.qml.session 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0
import xstudio.qml.global_store_model 1.0
import xstudio.qml.bookmarks 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.embedded_python 1.0

Item {

    Timer {
        id: callbackTimer
        function setTimeout(cb, delayTime) {
            callbackTimer.interval = delayTime;
            callbackTimer.repeat = false;
            callbackTimer.triggered.connect(cb);
            callbackTimer.triggered.connect(function release () {
                callbackTimer.triggered.disconnect(cb); // This is important
                callbackTimer.triggered.disconnect(release); // This is important as well
            });
            callbackTimer.start();
        }
    }
    property alias callbackTimer: callbackTimer

    EmbeddedPython {
        id: embeddedPython
        property string text: ""

        onBackendChanged: embeddedPython.createSession()
        onStdoutEvent: str => text += str
        onStderrEvent: str => text += str
    }
    property alias embeddedPython: embeddedPython

    XsPanelsLayoutModel {
        id: uiLayoutsModel
        property var root_index: uiLayoutsModel.index(-1, -1)
        property var numLayouts: 0
    }
    property alias uiLayoutsModel: uiLayoutsModel

    XsViewsModel {
        id: viewsModel
    }
    property alias viewsModel: viewsModel

    XsPopoutWindowsData {
        id: popoutWindowsModel
    }
    property alias popoutWindowsModel: popoutWindowsModel

    XsFilterModel {
        id: orderedPopoutWindowsModel
        sourceModel: popoutWindowsModel
        sortRoleName: "button_position"
        sortAscending: true
        onSourceModelChanged: sortRoleName = "button_position"
    }
    property alias orderedPopoutWindowsModel: orderedPopoutWindowsModel

    XsPlaylistsPanelBarExtraWidgetsModel {
        id: playlistsPanelBarExtraWidgets
    }
    property alias playlistsPanelBarExtraWidgets: playlistsPanelBarExtraWidgets

    XsSingletonItemsModel {
        id: singletonsModel
    }
    property alias singletonsModel: singletonsModel

    XsGlobalStoreModel {
        id: globalStoreModel
    }
    property alias globalStoreModel: globalStoreModel

    /* This model gives access to bookmark/notes data */
    XsBookmarkModel {
        id: bookmarkModel
        bookmarkActorAddr: sessionData.bookmarkActorAddr
    }
    property alias bookmarkModel: bookmarkModel


}