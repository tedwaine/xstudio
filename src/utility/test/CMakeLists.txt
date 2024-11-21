include(CTest)

SET(LINK_DEPS
	xstudio::bookmark
	caf::core
	xstudio::broadcast
)

create_tests("${LINK_DEPS}")


