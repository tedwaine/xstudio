import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQml.Models 2.14
import Qt.labs.qmlmodels 1.0

import xStudioReskin 1.0
import xstudio.qml.models 1.0

Item {

    id: menu_bar
    height: XsStyleSheet.menuHeight
   
    XsGradientRectangle{ id: bgDiv
        anchors.fill: parent
    }

    property string menu_model_name

    // Here we declare the menu model. Doing this creates an empty JsonTree 
    // in the backend with 'modelDataName' as its unqie identifier (so in this
    // case "shotgun menu bar" is the ID.... or, if the model with the 
    // given modelDataName already exists, then we connect to it.
    // The JsonTree desrcibes menu(s) and sub-menus - we can add to it and 
    // modfiy it from QML or the backend - we just need the ID to get to it.

    XsMenusModel {
        id: menus_model
        modelDataName: menu_model_name
        onJsonChanged: {
            root_index = index(-1, -1)
        }
    }

    // This index points us to the 'main menu bar' branch of
    // the global tree model
    property var root_index: menus_model.index(-1, -1)

    // classAll gets called by a menu item on its parent menu.
    function closeAll() {} 

    function hideOtherSubMenus(widget) {
        for (var i = 0; i < view.count; ++i) {
            let item = view.itemAtIndex(i)
            if (item != widget) {
                if (typeof item.hideSubMenus != "undefined") item.hideSubMenus()
                if (typeof item.hideOtherSubMenus != "undefined") item.hideOtherSubMenus(widget)
            }
        }
    }

    XsListView {

        id: view
        anchors.fill: parent
        orientation: ListView.Horizontal
        isScrollbarVisibile: false
        interactive: false

        model: DelegateModel {

            model: menus_model
            rootIndex: root_index

            delegate: chooser

            DelegateChooser {
                id: chooser
                role: "menu_item_type" 

                DelegateChoice {
                    roleValue: "menu"
                    
                    XsMenuItemNew {

                        menu_model: menus_model
        
                        // As we loop over the top level items in the 'main menu bar'
                        // here, we set the index to row=index, column=0. This takes
                        // us one step deeper into the tree on each iteration
                        menu_model_index: menus_model.index(index, 0, root_index)
        
                        // This var simply tells the pop-up to appear below the menu
                        // item rather than to the right of it.
                        is_in_bar: true
        
                        parent_menu: menu_bar
                        width: minWidth
        
                    }
                }

                DelegateChoice {
                    roleValue: "custom"
                    
                    XsMenuItemCustom { 
                        menu_model: menus_model
                        is_in_bar: true
                        menu_model_index: menus_model.index(index, 0, root_index)
                        parent_menu: menu_bar
                        width: minWidth
                    }
                    
                }
            }

        }

    }

}