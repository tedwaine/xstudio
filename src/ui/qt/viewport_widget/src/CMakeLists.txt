project(viewport_widget VERSION ${XSTUDIO_GLOBAL_VERSION} LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core Quick Gui Concurrent REQUIRED)

QT6_WRAP_CPP(OFFSCREEN_VIEWPORT_MOC_SRC
	"${ROOT_DIR}/include/xstudio/ui/qt/offscreen_viewport_base.hpp"
)

set(SOURCES
	offscreen_viewport_base.cpp	
	${OFFSCREEN_VIEWPORT_MOC_SRC})

if (XSTUDIO_OPEN_GL)
	find_package(Qt6 COMPONENTS OpenGL OpenGLWidgets REQUIRED)
	QT6_WRAP_CPP(VIEWPORT_OPENGL_UI_MOC_SRC
		"${ROOT_DIR}/include/xstudio/ui/qt/opengl/viewport_widget.hpp"
	)
	QT6_WRAP_CPP(OFFSCREEN_VIEWPORT_OPENGL_MOC_SRC
		"${ROOT_DIR}/include/xstudio/ui/qt/opengl/offscreen_viewport.hpp"
	)
	list(APPEND
		SOURCES
		${VIEWPORT_OPENGL_UI_MOC_SRC}
		${OFFSCREEN_VIEWPORT_OPENGL_MOC_SRC}
		opengl/offscreen_viewport.cpp
		opengl/viewport_widget.cpp
		)
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_library(xstudio::ui::qt::viewport_widget ALIAS ${PROJECT_NAME})

default_options_qt(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		CAF::core
		Qt6::Core
		Qt6::Qml
		Qt6::Quick
		xstudio::ui::opengl::viewport
		xstudio::ui::viewport
		xstudio::thumbnail
		xstudio::ui::qml::helper
)

if (XSTUDIO_OPEN_GL)
	target_link_libraries(${PROJECT_NAME}
		PUBLIC
			Qt6::OpenGLWidgets
			Qt6::OpenGL)
endif()
