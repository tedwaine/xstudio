
if(WIN32)
else()
find_package(OpenTime REQUIRED)
find_package(OpenTimelineIO REQUIRED)
find_package(Imath)
endif()


SET(LINK_DEPS
	xstudio::playhead
	xstudio::utility
	caf::core
	Imath::Imath
)

if(WIN32)
else()
SET(LINK_DEPS OTIO::opentime OTIO::opentimelineio)
endif()

SET(STATIC_LINK_DEPS
	xstudio::utility_static
	caf::core
	Imath::Imath
)

if(WIN32)
else()
SET(STATIC_LINK_DEPS OTIO::opentime OTIO::opentimelineio)
endif()

create_component_static(timeline 0.1.0 "${LINK_DEPS}" "${STATIC_LINK_DEPS}")

