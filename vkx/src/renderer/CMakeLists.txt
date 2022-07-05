target_sources(vkx
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/buffer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/image.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/model.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/viewport.cpp"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/core")