SET(LINK_DEPS
	CAF::core
    xstudio::ui::base
    xstudio::ui::canvas
    xstudio::ui::viewport
    xstudio::utility
    xstudio::media_reader
    xstudio::colour_pipeline
	OpenEXR::OpenEXR
	Imath::Imath
	"-framework Metal"
	"-framework AppKit"
)

find_package(OpenEXR)
find_package(Imath)

create_component_with_alias(metal_viewport xstudio::ui::graphics ${XSTUDIO_GLOBAL_VERSION} "${LINK_DEPS}")

if (USE_VCPKG)
	find_package(freetype CONFIG REQUIRED)
else()
	find_package(Freetype REQUIRED)
	target_include_directories(metal_viewport PUBLIC "${FREETYPE_INCLUDE_DIRS}")
endif()

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		freetype
)
