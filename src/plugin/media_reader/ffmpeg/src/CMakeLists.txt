project(media_reader_ffmpeg VERSION ${XSTUDIO_GLOBAL_VERSION} LANGUAGES CXX)

create_plugin_with_alias(
	media_reader_ffmpeg 
	xstudio::media_reader::ffmpeg 
	${XSTUDIO_GLOBAL_VERSION} 
	"${LINK_DEPS}"
)

target_compile_definitions(${PROJECT_NAME}
	PUBLIC OPTIMISED_BUFFER=1
)

if (${USE_VCPKG})

	find_package(FFMPEG REQUIRED)
	target_link_libraries(${PROJECT_NAME}
		PUBLIC
			xstudio::media_reader
		PRIVATE 
			${FFMPEG_LIBRARIES}
		)
	target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
	target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})

else()

	find_package(FFMPEG REQUIRED COMPONENTS avcodec avformat swscale avutil swresample)

	target_link_libraries(${PROJECT_NAME}
		PUBLIC
			xstudio::media_reader
			FFMPEG::avcodec
			FFMPEG::avformat
			FFMPEG::swscale
			FFMPEG::avutil
			FFMPEG::swresample
		)

endif()