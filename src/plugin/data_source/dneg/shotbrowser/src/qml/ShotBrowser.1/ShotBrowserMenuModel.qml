// SPDX-License-Identifier: Apache-2.0
import QtQuick 2.12

import xstudio.qml.models 1.0
import xstudio.qml.viewport 1.0

Item {

    // Note: For each instance of the ShotBrowser panel, we will have an
    // instance of THIS item. As such, the 'menu_model_name' needs to be
    // unique for each instance, so it has its own model data in the backend
    // from which the actual menu instance (of which there will also be
    // multiple instances) is built. See ShotBrowserPanel

    property var menu_model_name

    // Create a menu 'Some Menu' with an item in it that says 'Do Something'
    XsMenuModelItem {
        text: "Do Something"
        menuPath: "Some Menu"
        menuItemPosition: 1
        menuModelName: menu_model_name
        onActivated: {
            console.log("Doing Something in menu", menu_bar)
        }
    }

    XsMenuModelItem {
        text: "Do Something Else"
        menuPath: "Some Menu"
        menuItemPosition: 2
        menuModelName: menu_model_name
        onActivated: {
            console.log("Doing Something Else")
        }
    }

    // .. add a divider
    XsMenuModelItem {
        menuItemType: "divider"
        menuPath: "Some Menu"
        menuItemPosition: 3
        menuModelName: menu_model_name
    }

    XsMenuModelItem {
        text: "Do Some Other Thing"
        menuPath: "Some Menu"
        menuItemPosition: 5
        menuModelName: menu_model_name
        onActivated: {
            console.log("Doing Some Other Thing")
        }

        Component.onCompleted: {
            // we can set the menu position of a
            // menu ITEM using 'menuItemPosition'
            // However, this doesn't let us choose the ordering
            // of the sub menus that are specified by the 'menuPath'
            // property.
            //
            // For example, menuPath could be "Top Level|Next Level|Lower Level"
            // meaning that there are 3 nested submenus. We need some control
            // over how these submenus are ordered relative to other submenus
            // that are parented to the same menu.
            //
            // We can do that here like this:
            setMenuPathPosition("Some Menu", 0.0)
        }

    }

    // Declare a Hotkey. Note that hotkeys ultimately originate from a viewport.
    // If you move the mouse over a viewport it grabs the focus - then the
    // hotkey action will work. At
    XsHotkey {
        id: hk
        sequence: "Ctrl+G"
        name: "Do Some Deep Down Thing"
        description: "Does something"
        onActivated: {

            // N.B. We don't need this here because the menu item below is also
            // activated by this hotkey
            console.log("Some Deep Down Thing Hotkey Activated")
        }
    }

    // Create a menu 'Another Menu'
    XsMenuModelItem {
        text: "Some Deep Down Thing"
        menuPath: "Another Menu|Level1|Level2"
        menuItemPosition: 6
        menuModelName: menu_model_name

        // we can point to a hotkey - the hotkey sequence will be displayed
        // by the menu item. The nice thing about this is it's 'live' data
        // so if the hotkey is changed it will be reflected on the mnue
        hotkeyUuid: hk.uuid

        onActivated: {
            console.log("Some Deep Down Thing Activated")
        }
        Component.onCompleted: {
            // See comment in the 'Do Some Other Thing' menu item ... this
            // call ensures that the 'Another Menu' appears *after* the
            // 'Some Menu' sub menu.
            setMenuPathPosition("Another Menu", 1.0)
        }
    }

    // Pointless demo - but shows we can soon have a UI to set any/all hotkeys
    // from QML
    XsMenuModelItem {
        text: "Change the Hotkey"
        menuPath: "Another Menu|Level1|Level2"
        menuItemPosition: 7
        menuModelName: menu_model_name
        onActivated: {
            if (hk.sequence == "Ctrl+G") {
                hk.sequence = "Shift+Q"
            } else {
                hk.sequence = "Ctrl+G"
            }
        }
    }

    // this toggle changes the options in the multi choice menu that we
    // declare below
    XsMenuModelItem {
        text: "Switch Choices"
        menuPath: "Another Menu|Level1"
        menuItemType: "toggle"
        menuItemPosition: 0
        menuModelName: menu_model_name
        // for 'toggle' menus, use property 'isChecked' for check state
        onIsCheckedChanged: {
            if (isChecked) {
                multi_choice.choices = ["Ivy", "Shotgrid", "PFTRack", "Foo"]
            } else {
                multi_choice.choices = ["View", "Release", "Up", "Delete", "Forget", "Sorry"]
            }
        }
    }

    XsMenuModelItem {
        id: multi_choice
        text: "Multi Choice Example"
        menuPath: "Another Menu|Level1"
        menuItemType: "multichoice"
        menuItemPosition: 1
        choices: ["View", "Release", "Up", "Delete", "Forget", "Sorry"]
        currentChoice: "Release"
        menuModelName: menu_model_name

        // for 'multichoice' menus, use currentChoice property for the what's
        // selected
        onCurrentChoiceChanged: {
            console.log("currentChoice", currentChoice)
        }
    }

}