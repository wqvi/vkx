cmake_minimum_required(VERSION 3.0.0)
project(vkx
        LANGUAGES CXX
        VERSION 0.0.0
        )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vkx src/main.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/imgui.cpp)

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
                $<IF:$<TARGET_EXISTS:glm::glm>,glm::glm,glm>
                $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
                $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_include_directories(vkx
        PUBLIC
        	"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/stb"
                "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui")

# A precompiled header will speed up compile time due to the dummy thicc libraries included
target_precompile_headers(vkx
        PUBLIC
        "include/vkx/pch.hpp"
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
