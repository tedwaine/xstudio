project(grading VERSION ${XSTUDIO_GLOBAL_VERSION} LANGUAGES CXX)

SET(LINK_DEPS
	xstudio::module
	xstudio::plugin_manager
	xstudio::colour_pipeline
	xstudio::ui::graphics
	Imath::Imath
	OpenColorIO::OpenColorIO
	Qt6::Core
)

find_package(Imath)
find_package(OpenColorIO CONFIG)
find_package(Qt6 COMPONENTS Core REQUIRED)

create_plugin_with_alias(${PROJECT_NAME} xstudio::colour_pipeline::grading ${XSTUDIO_GLOBAL_VERSION}  "${LINK_DEPS}")

add_plugin_qml(${PROJECT_NAME} qml)
