import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.14
// import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0

XsViewerTextDisplay
{

    id: playheadPosition
    Layout.preferredWidth: textWidth + 8
    Layout.preferredHeight: parent.height
    modelDataName: playheadPosition + "_menu"
    menuWidth: 175
    property int selected: 0
    fontFamily: XsStyleSheet.fixedWidthFontFamily
    
    XsModelProperty {
        id: durationPref
        role: "valueRole"
        index: globalStoreModel.searchRecursive("/ui/qml/timeline_indicator", "pathRole")
    }
    property alias timelineIndicator: durationPref.value

    property var timelineUnits

    property int display_mode: timelineUnits == "Frames" || timelineUnits == "Frames From Timecode"? 0 : (timelineUnits == "Time" ? 1 : 2)

    XsMenuModelItem {
        menuPath: ""
        menuItemType: "radiogroup"
        menuItemPosition: 1
        choices: ["Duration", "Remaining", "FPS"]
        currentChoice: timelineIndicator
        menuModelName: playheadPosition + "_menu"
        onCurrentChoiceChanged: {
            if (currentChoice != timelineIndicator) {
                timelineIndicator = currentChoice                    
            }
            selected = choices.indexOf(currentChoice)
        }
        property var timelineIndicator_: timelineIndicator
        onTimelineIndicator_Changed: {
            if (timelineIndicator_ != undefined && currentChoice != timelineIndicator_)
                currentChoice = timelineIndicator_      
        }
    }

    property string fps: view.frame_rate_expr
    property var duration: {
        switch(display_mode) {
            case 0:
                return pad("" + viewportPlayhead.durationFrames, 4);
            case 1:
                return timeStr(viewportPlayhead.durationSeconds);
            case 2:
                return getTimeCodeStr(
                    viewportPlayhead.durationFrames,
                    Math.round(1.0/viewportPlayhead.frameRate));
        }
        return "--"
    }
    property string remaining: {
        switch(display_mode) {
            case 0:
                return pad("" + viewportPlayhead.durationFrames-viewportPlayhead.logicalFrame, 4);
            case 1:
                return timeStr(viewportPlayhead.durationSeconds - viewportPlayhead.positionSeconds);
            case 2:
                return getTimeCodeStr(
                    viewportPlayhead.durationFrames-viewportPlayhead.logicalFrame,
                    Math.round(1.0/viewportPlayhead.frameRate));
        }
        return "--"
    }
    

    text: selected == 0 ? duration : selected == 1 ? remaining : fps

    function pad(n, width, z) {
        z = z || '0';
        n = n + '';
        return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
    }

    function timeStr(tt) {
        var seconds = Math.floor(tt)
        var minutes = Math.floor(seconds / 60)
        var hours = Math.floor(minutes / 60)
        var SS = pad(seconds % 60, 2)
        var MM = pad(minutes % 60, 2)
        var str
        if (hours > 0) {
            var HH = hours % 24
            str = HH + ':' + MM + ':' + SS
        } else {
            str = MM + ':' + SS
        }
        return str
    }

    function getTimeCodeStr(displayValue, fps)
    {
        // e.g. minus 1 so that frame 30 for 30fps should still be 00:00
        // displayValue = displayValue
        var frames = displayValue % Math.round(fps)
        var seconds = Math.floor(displayValue / fps)
        var minutes = Math.floor(seconds / 60)
        var hours = Math.floor(minutes / 60)
        var FF = pad(frames, 2)
        var SS = pad(seconds % 60, 2)
        var MM = pad(minutes % 60, 2)
        var HH = pad(hours, 2)
        return HH + ':' + MM + ':' + SS + ':' + FF
    }
}