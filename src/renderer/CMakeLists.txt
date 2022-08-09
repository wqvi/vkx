target_sources(vkx
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/image.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/model.cpp"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/core")