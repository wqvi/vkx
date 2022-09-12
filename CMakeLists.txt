cmake_minimum_required(VERSION 3.0.0)
project(vkx
        LANGUAGES CXX
        VERSION 0.0.0
        )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vkx src/main.cpp)

add_subdirectory("src")
add_subdirectory("lib/VulkanMemoryAllocator")

set_target_properties(vkx PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1)

find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(vkx
	PUBLIC
		Vulkan::Vulkan
		VulkanMemoryAllocator
		glm
		SDL2
		)

target_include_directories(vkx
        PUBLIC
        	"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")

# A precompiled header will speed up compile time due to the dummy thicc libraries included
target_precompile_headers(vkx
        PUBLIC
        "include/vkx/pch.hpp"
        )

# Compile vertex shader to SPIR-V
add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader.vert" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv"
        )

# Compile fragment shader to SPIR-V
add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader.frag" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv"
        )

# Move compiled vertex shader SPIR-V to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv"
        $<TARGET_FILE_DIR:vkx>
        )

# Move compiled fragment shader SPIR-V to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv"
        $<TARGET_FILE_DIR:vkx>
        )

# Move image to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/resources/a.jpg"
        $<TARGET_FILE_DIR:vkx>
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
