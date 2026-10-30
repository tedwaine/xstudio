include(CTest)

SET(LINK_DEPS
	xstudio::ui::qml::helper
	xstudio::ui::qml::studio
)

create_test_with_qml("${CMAKE_CURRENT_SOURCE_DIR}/model_data_ui_test.cpp" "${LINK_DEPS}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ui/qml/xstudio/qml.qrc")

