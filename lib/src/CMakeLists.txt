target_sources(VoxelEngine
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/window.cpp"
)

message(STATUS "Including vkx source directory!")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/voxels")