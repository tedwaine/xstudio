include(CTest)

SET(LINK_DEPS
    xstudio::ui::qml::session
    xstudio::ui::qml::helper
    Qt6::Core
    Qt6::Gui
    Qt6::Test
)

create_tests("${LINK_DEPS}")
