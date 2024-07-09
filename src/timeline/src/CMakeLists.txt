## N.B. We don't have OTIO support on Windows yet. Perhaps the build
## complexity for OTIO C++ lib is a problem?

if(WIN32)
else()
find_package(OpenTime REQUIRED)
find_package(OpenTimelineIO REQUIRED)
find_package(Imath)
endif()

find_package(fmt REQUIRED)

if(WIN32)
	SET(LINK_DEPS
		xstudio::playhead
		xstudio::utility
		caf::core
		Imath::Imath
		fmt::fmt
	)
else()
	SET(LINK_DEPS
		xstudio::playhead
		xstudio::utility
		caf::core
		Imath::Imath
		fmt::fmt
		OTIO::opentime
		OTIO::opentimelineio
	)
endif()

SET(STATIC_LINK_DEPS
	xstudio::utility_static
	caf::core
	Imath::Imath
	fmt::fmt
)

if(WIN32)
	SET(STATIC_LINK_DEPS
		xstudio::utility_static
		caf::core
		Imath::Imath
	)
else()
	SET(STATIC_LINK_DEPS
		xstudio::utility_static
		caf::core
		Imath::Imath
		OTIO::opentime
		OTIO::opentimelineio
	)
endif()

create_component_static(timeline 0.1.0 "${LINK_DEPS}" "${STATIC_LINK_DEPS}")

