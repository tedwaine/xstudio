find_package(Qt6 COMPONENTS Pdf Gui Core Widgets Quick Concurrent REQUIRED)

SET(LINK_DEPS
	xstudio::media_reader
)

create_plugin_with_alias(media_reader_pdf xstudio::media_reader::pdf ${XSTUDIO_GLOBAL_VERSION}  "${LINK_DEPS}")

target_link_libraries(media_reader_pdf
	PUBLIC
		xstudio::utility
	PRIVATE
		Qt6::Pdf
		Qt6::Core
		Qt6::Widgets
		Qt6::Quick
		Qt6::Concurrent
)
