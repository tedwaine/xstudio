SET(LINK_DEPS
	CAF::core
	Qt6::Core
	Qt6::Qml
	Qt6::Gui
	Qt6::Quick
	Qt6::Widgets
	xstudio::global_store
	xstudio::media
	xstudio::utility
)

if (${XSTUDIO_GRAPHICS_API} STREQUAL "OpenGL")
	find_package(Qt6 COMPONENTS OpenGL REQUIRED)
	list(APPEND LINK_DEPS Qt6::OpenGL)
endif()

if (NOT WIN32)
	list(APPEND LINK_DEPS Qt6::DBus)
endif()

SET(EXTRAMOC
	# "${ROOT_DIR}/include/xstudio/ui/qml/thumbnail_provider_ui.hpp"
	# "${ROOT_DIR}/include/xstudio/ui/qml/shotgun_provider_ui.hpp"
	"${ROOT_DIR}/include/xstudio/ui/qml/QTreeModelToTableModel.hpp"
	"${ROOT_DIR}/include/xstudio/ui/qml/json_tree_model_ui.hpp"
	"${ROOT_DIR}/include/xstudio/ui/qml/model_data_ui.hpp"
	"${ROOT_DIR}/include/xstudio/ui/qml/module_data_ui.hpp"
	"${ROOT_DIR}/include/xstudio/ui/qml/actor_object.hpp"
)

create_qml_component(helper ${XSTUDIO_GLOBAL_VERSION} "${LINK_DEPS}" "${EXTRAMOC}")
