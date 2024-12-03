include(CTest)

SET(LINK_DEPS
	xstudio::utility
	xstudio::broadcast
)

create_tests("${LINK_DEPS}")

