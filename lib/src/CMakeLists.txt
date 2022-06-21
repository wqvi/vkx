target_sources(VoxelEngine
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/window.cpp"
)

add_subdirectory("renderer")
add_subdirectory("voxels")