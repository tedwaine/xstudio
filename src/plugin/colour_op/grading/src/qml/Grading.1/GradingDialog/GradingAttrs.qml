// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.15

import xstudio.qml.models 1.0

import MaskTool 1.0
import xStudioReskin 1.0

Item {

    /* This connects to the backend model data named grading_settings to which
    many of our attributes have been added*/
    XsModuleData {
        id: grading_tool_attrs_data
        modelDataName: "grading_settings"
    }

    ///////////////////////////////////////////////////////////////////////
    // to DIRECTLY expose attribute role data we use XsAttributeValue and 
    // give it the title (name) of the attribute. By default it will expose 
    // the 'value' role data of the attribute but you can override this to 
    // get to other role datas such as 'combo_box_options' (the string 
    // choices in a StringChoiceAttribute) or 'default_value' etc.

    XsAttributeValue {
        id: __grading_tool_active
        attributeTitle: "grading_tool_active"
        model: grading_tool_attrs_data
    }
    property alias grading_tool_active: __grading_tool_active.value

    XsAttributeValue {
        id: __grading_panel
        attributeTitle: "grading_panel"
        model: grading_tool_attrs_data
    }
    property alias grading_panel: __grading_panel.value

    XsAttributeValue {
        id: __grading_panel_options
        attributeTitle: "grading_panel"
        role: "combo_box_options"
        model: grading_tool_attrs_data
    }
    property alias grading_panel_options: __grading_panel_options.value

    XsAttributeValue {
        id: __grading_action
        attributeTitle: "grading_action"
        model: grading_tool_attrs_data
    }
    property alias grading_action: __grading_action.value

    XsAttributeValue {
        id: __grading_tracking
        attributeTitle: "grading_tracking"
        model: grading_tool_attrs_data
    }
    property alias grading_tracking: __grading_tracking.value

    XsAttributeValue {
        id: __media_colour_managed
        attributeTitle: "media_colour_managed"
        model: grading_tool_attrs_data
    }
    property alias media_colour_managed: __media_colour_managed.value

    XsAttributeValue {
        id: __grading_bookmark
        attributeTitle: "grading_bookmark"
        model: grading_tool_attrs_data
    }
    property alias grading_bookmark: __grading_bookmark.value

    XsAttributeValue {
        id: __tool_panel
        attributeTitle: "tool_panel"
        model: grading_tool_attrs_data
    }
    property alias tool_panel: __tool_panel.value

    XsAttributeValue {
        id: __drawing_bypass
        attributeTitle: "drawing_bypass"
        model: grading_tool_attrs_data
    }
    property alias drawing_bypass: __drawing_bypass.value

    XsAttributeValue {
        id: __grade_active
        attributeTitle: "grade_active"
        model: grading_tool_attrs_data
    }
    property alias grade_active: __grade_active.value

    XsAttributeValue {
        id: __working_space
        attributeTitle: "working_space"
        model: grading_tool_attrs_data
    }
    property alias working_space: __working_space.value

    XsAttributeValue {
        id: __colour_space
        attributeTitle: "colour_space"
        model: grading_tool_attrs_data
    }
    property alias colour_space: __colour_space.value

    XsAttributeValue {
        id: __grade_in
        attributeTitle: "grade_in"
        model: grading_tool_attrs_data
    }
    property alias grade_in: __grade_in.value

    XsAttributeValue {
        id: __grade_out
        attributeTitle: "grade_out"
        model: grading_tool_attrs_data
    }
    property alias grade_out: __grade_out.value

    XsAttributeValue {
        id: __mask_tool_active
        attributeTitle: "mask_tool_active"
        model: grading_tool_attrs_data
    }
    property alias mask_tool_active: __mask_tool_active.value

    ///////////////////////////////////////////////////////////////////////
    // Get access to all 'role' data items of the actual grading attributes
    // The 'values' object has properties that map to the names of attribute
    // role data. E.g. 'value' 'default_value' 'float_scrub_min' 'combo_box_options'
    // etc.

    XsAttributeFullData {
        id: __slope
        attributeTitle: "Slope"
        model: grading_tool_attrs_data
    }
    property alias slope: __slope.values

    XsAttributeFullData {
        id: __offset
        attributeTitle: "Offset"
        model: grading_tool_attrs_data
    }
    property alias offset: __offset.values

    XsAttributeFullData {
        id: __power
        attributeTitle: "Power"
        model: grading_tool_attrs_data
    }
    property alias power: __power.values

    XsAttributeFullData {
        id: __saturation
        attributeTitle: "Saturation"
        model: grading_tool_attrs_data
    }
    property alias saturation: __saturation.values

    ///////////////////////////////////////////////////////////////////////
    // this provides access to the attributes in the "grading_sliders" group
    XsModuleData {
        id: grading_sliders_model
        modelDataName: "grading_sliders"
    }
    property alias grading_sliders_model: grading_sliders_model

    ///////////////////////////////////////////////////////////////////////
    // helpers
    function getAttrValue(attr_name) {
        var idx = grading_sliders_model.searchRecursive(attr_name,"title")
        if (idx.valid) {
            return grading_sliders_model.get(idx, "value")
        }
        console.log("GradingAttrs: attribute named", attr_name, "not found.")
        return undefined
    }

}