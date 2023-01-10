target_sources(vkx
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/buffers.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/image.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/model.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/swapchain.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/core")