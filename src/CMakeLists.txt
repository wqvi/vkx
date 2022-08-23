target_sources(vkx
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/raycast.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/sdl.cpp"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer")