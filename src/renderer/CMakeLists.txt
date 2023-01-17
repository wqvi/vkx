target_sources(vkx
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/buffers.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/commands.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/image.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/model.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/pipeline.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/queue_config.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/swapchain.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/swapchain_info.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/sync_objects.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/vertex.cpp"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/memory")