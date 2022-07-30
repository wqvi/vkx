cmake_minimum_required(VERSION 3.0.0)
project(vkx
        LANGUAGES CXX
        VERSION 0.0.0
        )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vkx src/main.cpp)

add_subdirectory("src")

set_target_properties(vkx PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1)

if (GCC)
    # All the warnings!
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wcast-align
            -Wcast-qual
            -Wfloat-equal
            -Wformat=2
            -Wlogical-op
            -Wmissing-declarations
            -Wmissing-include-dirs
            -Wpointer-arith
            -Wredundant-decls
            -Wsequence-point
            -Wshadow
            -Wswitch
            -Wundef
            -Wunreachable-code
            -Wunused-but-set-parameter
            -Wwrite-strings
            )
endif ()

find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(vkx
	PUBLIC
		Vulkan::Vulkan
		glm
		SDL2
		)

target_include_directories(vkx
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

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

add_custom_command(TARGET vkx PRE_BUILD
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader.comp" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.comp.spv"
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

add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.comp.spv"
        $<TARGET_FILE_DIR:vkx>
        )

# Move image to build directory
add_custom_command(TARGET vkx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/resources/a.jpg"
        $<TARGET_FILE_DIR:vkx>
        )
