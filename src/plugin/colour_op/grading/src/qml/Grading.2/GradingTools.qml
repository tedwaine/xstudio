// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQml.Models 2.14
import QtQuick.Dialogs 1.3 //for ColorDialog
import QtGraphicalEffects 1.15 //for RadialGradient
import Qt.labs.qmlmodels 1.0 //for RadialGradient

import xStudioReskin 1.0
import xstudio.qml.bookmarks 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

Item { id: dialog
    anchors.fill: parent

    GTAttributes { id: attrs }

    MAttributes { id: mask_attrs }

    property real itemSpacing: 1
    property real buttonSpacing: 1
    property real btnWidth: XsStyleSheet.primaryButtonStdWidth
    property real btnHeight: XsStyleSheet.widgetStdHeight + 4
    property real panelPadding: XsStyleSheet.panelPadding
    property color panelColor: XsStyleSheet.panelBgColor

    property alias grading_sliders_model: attrs.grading_sliders_model
    property alias grading_wheels_model: attrs.grading_wheels_model

    onVisibleChanged: {
        if (visible) {
            attrs.grading_tool_active = true
        } else {
            attrs.grading_tool_active = false
        }
    }

    function hasActiveGrade() {
        return attrs.grading_bookmark && attrs.grading_bookmark != "00000000-0000-0000-0000-000000000000"
    }

    XsBookmarkFilterModel {
        id: bookmarkFilterModel
        sourceModel: bookmarkModel
        currentMedia: currentPlayhead.mediaUuid
        showHidden: true
        sortbyCreated: true
    }

    XsSplitView {
        anchors.fill: parent

        GTLeftSection{ id: leftSection
            SplitView.preferredWidth: 300
            SplitView.fillHeight: true
        }
        GTRightSection{
            SplitView.fillWidth: true
            SplitView.fillHeight: true
        }
    }
}