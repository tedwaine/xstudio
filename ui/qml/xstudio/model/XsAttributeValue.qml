import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

XsModelProperty {

    role: "value"
    property var model
    property string attributeTitle: ""
    index: model ? model.searchRecursive(attributeTitle, "title") : null
    property var modelLength: model ? model.length : 0
    signal indexBecameValid()
    onAttributeTitleChanged: {
        update_index()
    }

    onModelLengthChanged: {
        if (modelLength > 0) console.log("Model length changed, new length:", modelLength)
        update_index()
    }

    function update_index() {
        if (model) {
            var was_valid = index.valid
            index = model.searchRecursive(attributeTitle, "title")
            if (!was_valid && index.valid) {
                console.log("Index became valid for attributeTitle:", attributeTitle)
                indexBecameValid()
            }
        }
    }

}