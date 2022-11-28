target_sources(vkx
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/aabb.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/events.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/raycast.cpp"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer")