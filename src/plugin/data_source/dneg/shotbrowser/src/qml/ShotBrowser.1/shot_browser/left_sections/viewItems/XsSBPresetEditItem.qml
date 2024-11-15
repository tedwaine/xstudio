// SPDX-License-Identifier: Apache-2.0

import QtQuick

import QtQuick.Layouts


import xStudio 1.0
import ShotBrowser 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.models 1.0

Rectangle{
    id: thisItem

    color: XsStyleSheet.widgetBgNormalColor

    property var delegateModel: null
    property var termModel: []
    property var termValueRole: valueRole

    onTermValueRoleChanged: {
        if(valueBox.count && valueBox.currentText != valueRole) {
            let i = valueBox.find(valueRole)
            if(i != -1)
                valueBox.currentIndex = i
            else {
                valueBox.editText = valueRole
            }
        }
    }
    function setTermValue(value) {
        valueRole = value
    }

    Item{ id: rowItems
        anchors.fill: parent

        DropArea {
            Rectangle{id: rect; anchors.fill: parent; color: "yellow"; opacity: parent.containsDrag?1:0.3; visible: dragBtn.enabled}

            keys: ["PRESET-TERM"]
            width: parent.width
            height: parent.height - 1
            onEntered: {
                // console.log("DropArea: Entered")
                if(rect.visible){
                    rect.color = "red"
                }
            }
            onExited: {
                // console.log("DropArea: Exited")
                if(rect.visible){
                    rect.color = "yellow"
                }
            }
            onDropped: {
                // console.log(drop)
            }
        }

        Drag.active: dragArea.held
        Drag.source: dragArea
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.keys: ["PRESET-TERM"]

        // states: State {
        //     when: dragArea.held
        //     ParentChange { target: content; parent: presetsDiv }
        //     // AnchorChanges {
        //     //     target: content
        //     //     anchors { horizontalCenter: undefined; verticalCenter: undefined }
        //     // }
        // }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10 * delegateModel.notifyModel.depthAtRow(index)
            spacing: 1

            XsPrimaryButton{ id: dragBtn
                enabled: false //#TODO
                Layout.preferredWidth: dragWidth
                Layout.fillHeight: true
                imgSrc: "qrc:///shotbrowser_icons/drag_indicator.svg"
                isActiveViaIndicator: false
                isActive: dragArea.held
                onPressed: {
                    dragArea.held = true
                }
                onReleased: {
                    dragArea.held = false
                }

                MouseArea{ id: dragArea
                    property bool held: false
                    width: parent.width
                    height: parent.height
                    drag.target: held? rowItems : undefined
                    drag.axis: Drag.YAxis
                    onPressed: held = true
                    onReleased: held = false
                }
            }

            XsCheckBox { id: checkBox
                Layout.preferredWidth: enableWidth
                Layout.fillHeight: true
                checked: enabledRole
                onClicked: enabledRole = !enabledRole
            }

            XsPrimaryButton{ id: insertButton
                Layout.preferredWidth: btnWidth/2
                Layout.fillHeight: true
                imgSrc: "qrc:///shotbrowser_icons/arrow_right.svg"
                isActiveViaIndicator: true
                visible: termRole == "Operator"
                isActive: delegateModel.newTermParent == delegateModel.notifyModel.mapRowToModel(index)
                onClicked: {
                    delegateModel.model.expandRow(index)
                    if(isActive) {
                        delegateModel.newTermParent = presetIndex
                    } else {
                        delegateModel.newTermParent = delegateModel.model.mapRowToModel(index)
                    }
                }
            }

            XsComboBox {
                // Layout.fillWidth: true
                Layout.preferredWidth: termWidth - (termRole == "Operator" ? btnWidth/2 : 0)
                Layout.fillHeight: true
                model: termModel
                enabled: enabledRole

                currentIndex: this.count ? find(termRole) : -1

                onActivated: (aindex) => {
                    if(termRole != textAt(aindex)) {
                        let ti = thisItem.delegateModel.model.mapRowToModel(index)
                        let r = ti.row+1

                        ShotBrowserEngine.presetsModel.insertTerm(textAt(aindex), ti.row, ti.parent)
                        ShotBrowserEngine.presetsModel.removeRows(r, 1, ti.parent)
                    }
                }
            }
            Item{ id: equationDiv
                Layout.preferredWidth: modeWidth
                Layout.fillHeight: true

                XsPrimaryButton{ id: equationBtn
                    property bool isLiveLink: livelinkRole != undefined && livelinkRole
                    property bool isNegate: negatedRole != undefined && negatedRole
                    property bool isEqual: !isLiveLink && !isNegate
                    property bool equalMenuEnabled: !isEqual

                    anchors.fill: parent
                    imgSrc:
                        isLiveLink? "qrc:/icons/link.svg" :
                        isNegate? "qrc:/shotbrowser_icons/exclamation.svg" :
                        "qrc:/shotbrowser_icons/equal.svg"
                    isActiveViaIndicator: false
                    enabled: enabledRole
                    isActive: !isEqual
                    onClicked:{
                        if(equationMenu.visible)
                            equationMenu.visible = false
                        else {
                            equationMenu.showMenu(
                                equationBtn,
                                width/2,
                                height/2);
                        }
                    }
                }
            }
            XsComboBoxEditable {
                id: valueBox
                Layout.fillWidth: true
                Layout.preferredWidth: btnWidth*4.2
                Layout.fillHeight: true
                model: ShotBrowserEngine.presetsModel.termModel(termRole, entityType, projectId)
                enabled: enabledRole && !livelinkRole
                textRole: "nameRole"
                currentIndex: -1

                onCountChanged: {
                    if(count && currentText != termValueRole) {
                        let i = find(termValueRole)
                        if(i != -1) {
                            currentIndex = i
                        }
                        else {
                            // inject value into model.
                            model.insertRowsData(0,1, model.index(-1,-1), {"name": termValueRole})
                            currentIndex = 0
                        }
                    }
                }

                onAccepted: {
                    if (find(editText) === -1) {
                        setTermValue(editText)
                        model.insertRowsData(0, 1, model.index(-1,-1), {"name": editText})
                        currentIndex = 0
                    } else {
                        currentIndex = find(editText)
                        setTermValue(textAt(currentIndex))
                    }
                    focus = false
                }

                onActivated: (aindex) => {
                    setTermValue(textAt(aindex))
                }
            }
            XsPrimaryButton{
                Layout.preferredWidth: closeWidth
                Layout.fillHeight: true
                imgSrc: "qrc:/icons/close.svg"
                onClicked: {
                    // map to real model..
                    let i = delegateModel.model.mapRowToModel(index)
                    delegateModel.model.model.removeRows(i.row, 1, i.parent)
                }
            }

        }


    }



    XsPopupMenu {
        id: equationMenu
        visible: false
        menu_model_name: "equationMenu"+thisItem

        XsMenuModelItem {
            text: "Equals"
            menuPath: ""
            menuModelName: equationMenu.menu_model_name
            enabled: equationBtn.equalMenuEnabled
            onActivated: {
                equationBtn.equalMenuEnabled = false

                if(livelinkRole != undefined)
                    livelinkRole = false
                if(negatedRole != undefined)
                    negatedRole = false
            }
        }
        XsMenuModelItem {
            text: "Negates"
            menuPath: ""
            menuModelName: equationMenu.menu_model_name
            enabled: negatedRole != undefined && !negatedRole
            onActivated: {
                equationBtn.equalMenuEnabled = true

                if(livelinkRole != undefined)
                    livelinkRole = false
               negatedRole = true
            }
        }
        XsMenuModelItem {
            text: "Live Link"
            menuPath: ""
            menuModelName: equationMenu.menu_model_name
            enabled: livelinkRole != undefined && !livelinkRole
            onActivated: {
                equationBtn.equalMenuEnabled = true

                livelinkRole = true
                if(negatedRole != undefined)
                    negatedRole = false
            }
        }
    }
}

