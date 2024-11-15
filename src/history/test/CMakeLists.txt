include(CTest)

SET(LINK_DEPS
	xstudio::utility
	xstudio::broadcast
	xstudio::media
)

create_tests("${LINK_DEPS}")

