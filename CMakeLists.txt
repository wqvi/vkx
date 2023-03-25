cmake_minimum_required(VERSION 3.25.0)
project(vkx LANGUAGES CXX VERSION 0.0.0)

add_executable(vkx src/main.cpp)

set_target_properties(vkx 
    PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

add_subdirectory("src")

set_target_properties(vkx PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)

find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)
find_package(SDL3 REQUIRED)
find_package(glm CONFIG REQUIRED)

find_path(VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIRS "vk_mem_alloc.h")

target_link_libraries(vkx 
    PUBLIC
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        SDL3::SDL3
        $<IF:$<TARGET_EXISTS:glm::glm>, glm::glm, glm>
)

target_include_directories(vkx 
    PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/include" 
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb"
        ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIRS}
)

if (WIN32)
    add_custom_command(TARGET vkx POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:vkx> $<TARGET_FILE_DIR:vkx>
        COMMAND_EXPAND_LISTS
    )
endif(WIN32)

# A precompiled header will speed up compile time due to the dummy thicc libraries included
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
