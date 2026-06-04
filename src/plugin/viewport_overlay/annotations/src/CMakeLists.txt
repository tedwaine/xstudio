find_package(Qt6 COMPONENTS Core Quick Gui REQUIRED)
find_package(Imath)

file(GLOB EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/serialisers/*/*.cpp)

qt6_add_resources(EXTRA_SOURCES qml/AnnotationsUI.2/anno_tools.qrc)
 
SET(LINK_DEPS
	xstudio::module
	xstudio::plugin_manager
	xstudio::ui::opengl::viewport
	Imath::Imath
	Qt6::Core
)

create_plugin_with_alias(annotations_tool xstudio::viewport::annotations_tool ${XSTUDIO_GLOBAL_VERSION}  "${LINK_DEPS}" ${EXTRA_SOURCES})

add_plugin_qml(${PROJECT_NAME} qml)
