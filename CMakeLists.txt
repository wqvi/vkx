cmake_minimum_required(VERSION 3.25.0)
project(vkx LANGUAGES CXX VERSION 0.0.0)

add_executable(vkx
	src/application.cpp
	src/camera.cpp
	src/main.cpp
	src/raycast.cpp
	src/renderer/allocator.cpp
	src/renderer/buffers.cpp
	src/renderer/commands.cpp
	src/renderer/image.cpp
	src/renderer/model.cpp
	src/renderer/pipeline.cpp
	src/renderer/queue_config.cpp
	src/renderer/renderer.cpp
	src/renderer/swapchain.cpp
	src/renderer/swapchain_info.cpp
	src/renderer/sync_objects.cpp
	src/renderer/texture.cpp
	src/renderer/vertex.cpp
	src/voxels/voxels.cpp
	src/window.cpp
	)

set_target_properties(vkx 
    PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

set_target_properties(vkx PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)

find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(vkx 
    PUBLIC
        Vulkan::Vulkan
        Vulkan::shaderc_combined
	GPUOpen::VulkanMemoryAllocator
        $<IF:$<TARGET_EXISTS:glm::glm>, glm::glm, glm>
	SDL2::SDL2
)

target_include_directories(vkx 
    PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/include" 
)

target_precompile_headers(vkx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/vkx/pch.hpp")

# Move image to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/resources/a.jpg" $<TARGET_FILE_DIR:vkx>
)

# 2d shaders

# Compile vertex shader to SPIR-V
add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/highlight.vert" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/highlight.vert.spv"
        )

# Compile fragment shader to SPIR-V
add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/highlight.frag" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/highlight.frag.spv"
        )

# Move compiled vertex shader SPIR-V to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/highlight.vert.spv"
        $<TARGET_FILE_DIR:vkx>
        )

# Move compiled fragment shader SPIR-V to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/highlight.frag.spv"
        $<TARGET_FILE_DIR:vkx>
        )

# Compile vertex shader to SPIR-V
add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader2D.vert" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader2D.vert.spv"
        )

# Compile fragment shader to SPIR-V
add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader2D.frag" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader2D.frag.spv"
        )

# Move compiled vertex shader SPIR-V to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader2D.vert.spv"
        $<TARGET_FILE_DIR:vkx>
        )

# Move compiled fragment shader SPIR-V to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader2D.frag.spv"
        $<TARGET_FILE_DIR:vkx>
        )

# Compile compute shader to SPIR-V
add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/greedy.comp" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/greedy.comp.spv"
        )

# Move compiled fragment shader SPIR-V to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/greedy.comp.spv"
        $<TARGET_FILE_DIR:vkx>
        )
