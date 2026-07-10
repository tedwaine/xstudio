include(CTest)

SET(LINK_DEPS
	xstudio::ui::viewport
	xstudio::ui::graphics
	Imath::Imath
)

find_package(Imath)

create_tests("${LINK_DEPS}")

