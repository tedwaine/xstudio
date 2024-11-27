find_package(Imath REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenTime REQUIRED)
find_package(OpenTimelineIO REQUIRED)

SET(LINK_DEPS
	xstudio::utility
	xstudio::media
	xstudio::playhead
	caf::core
	Imath::Imath
	fmt::fmt
	OTIO::opentime
	OTIO::opentimelineio
)

SET(STATIC_LINK_DEPS
	xstudio::utility_static
	xstudio::media_static
	caf::core
	Imath::Imath
	fmt::fmt
	OTIO::opentime
	OTIO::opentimelineio
)

create_component_static(timeline 0.1.0 "${LINK_DEPS}" "${STATIC_LINK_DEPS}")

