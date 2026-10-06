SET(LINK_DEPS
	CAF::core
	Imath::Imath
	Qt6::Core
	Qt6::Qml
	Qt6::Quick
	xstudio::module
	xstudio::playhead
	xstudio::ui::graphics
	xstudio::ui::viewport
	xstudio::ui::qml::helper
	xstudio::utility
)

if (${XSTUDIO_GRAPHICS_API} STREQUAL "Metal")
	LIST(APPEND LINK_DEPS "-framework QuartzCore")
endif()

SET(EXTRAMOC
	"${ROOT_DIR}/include/xstudio/ui/qml/hotkey_ui.hpp"
	"${ROOT_DIR}/include/xstudio/ui/qml/qml_viewport.hpp"
	"${ROOT_DIR}/include/xstudio/ui/qml/qml_viewport_renderer.hpp"
)

find_package(Imath)

create_qml_component(viewport ${XSTUDIO_GLOBAL_VERSION} "${LINK_DEPS}" "${EXTRAMOC}")
