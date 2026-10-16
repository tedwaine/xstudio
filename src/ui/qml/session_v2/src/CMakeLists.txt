SET(LINK_DEPS
	CAF::core
	Qt6::Core
	Qt6::Test
	xstudio::ui::qml::helper
	xstudio::timeline
	xstudio::utility
	xstudio::session
)

SET(EXTRAMOC
	"${ROOT_DIR}/include/xstudio/ui/qml/new_session_model_ui.hpp"
)

create_qml_component(session_v2 ${XSTUDIO_GLOBAL_VERSION} "${LINK_DEPS}" "${EXTRAMOC}")
