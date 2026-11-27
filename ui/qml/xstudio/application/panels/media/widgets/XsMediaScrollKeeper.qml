// SPDX-License-Identifier: Apache-2.0
import QtQuick

import xStudio 1.0

Item {
    id: widget
    property string prefix: ""

    //
    // Here is the logic to remember the scrolled position per container.
    // This assumes that somewhere in the parent hierarchy is a dictionary
    // property named scrollKeeper into which the scroll data is kept.
    //

    // machanism to stash the contentY before it gets erased
    // when a new container is selected
    property real storeContentY: 0

    property var watcher: prefix+inspectedContainerActor
    property var previous
    onWatcherChanged: storeAndRestoreScrollPosition()

    function storeAndRestoreScrollPosition() {
        if (!inspectedContainerActor)
            return
        if (!previous)
            previous = inspectedContainerActor
        scrollKeeper[previous] = contentY
        if (typeof scrollKeeper == "object" && scrollKeeper.hasOwnProperty(inspectedContainerActor)) {
             callbackTimer.setTimeout(function() { return function() {
                contentY = scrollKeeper[inspectedContainerActor]
            }}(), 100);
        }
        previous = inspectedContainerActor        
    }

    // machanisms to store and restore the scroll position
    // whenever the mode is changed between grid and list
    Component.onDestruction: storeAndRestoreScrollPosition()
    Component.onCompleted: storeAndRestoreScrollPosition()
}
