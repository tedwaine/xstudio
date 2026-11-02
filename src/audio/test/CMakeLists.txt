include(CTest)

if(WIN32)
    # Additional Windows-specific configuration here.
elseif(APPLE)
    # TODO: Apple-specific configuration here.
else()
    find_package(ALSA REQUIRED)
    find_package(PulseAudio REQUIRED)
endif()

SET(LINK_DEPS
	xstudio::audio_output
	xstudio::playhead
	xstudio::global
)

if(WIN32)
    # Link against Windows Core Audio libraries.
    list(APPEND LINK_DEPS PUBLIC "avrt.lib" "mmdevapi.lib")
elseif(APPLE)
	list(APPEND LINK_DEPS "-framework CoreFoundation" "-framework AudioToolbox")
else()
    list(APPEND LINK_DEPS PUBLIC pulse pulse-simple)
endif()

create_tests("${LINK_DEPS}")

