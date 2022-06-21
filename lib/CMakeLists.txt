project(VoxelEngine
    LANGUAGES CXX
    VERSION 0.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
	"src/main.cpp"
)

add_subdirectory("src")

if (MSVC)
	#[[
	Weird MSVC Visual Studio IDE work around.
	This is to get the main executable as a startup project.
	]]
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
	#[[ 
	Another weird MSVC Visual Studio IDE work around.
	This allows for IDE specific filter usage.
	]]
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	# Set MSVC compiler warning level to the highest
	target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
	# Set Clang and GCC warning level to the highest
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic) 
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

if (MSVC)
	target_link_libraries(${PROJECT_NAME}
		PRIVATE
			Vulkan::Vulkan
			glfw
			glm::glm
		)
else()
	target_link_libraries(${PROJECT_NAME}
			PRIVATE
				Vulkan::Vulkan
				glfw
				glm
			)
endif()

# This is temporary
target_include_directories(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# A precompiled header will speed up compile time due to the dummy thicc libraries included
target_precompile_headers(${PROJECT_NAME} 
	PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp"
)


if (MSVC)
	#[[
	Silly script to recursively create a filter structure for MSVC Visual Studio IDE
	Don't touch this it just works.
	This needs to be after we add the executable and add subdirectories!
	]]
	get_target_property(SRCS ${PROJECT_NAME} SOURCES)
	foreach(FILE ${SRCS})
	    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
	    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
	    string(REPLACE "/" "\\" GROUP "${GROUP}")
	    source_group("${GROUP}" FILES "${FILE}")
	endforeach()
endif()

# Compile vertex shader to SPIR-V
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader.vert" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv"
)

# Compile fragment shader to SPIR-V
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader.frag" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv"
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/shader.comp" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.comp.spv"
)

# Move compiled vertex shader SPIR-V to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv"
	$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Move compiled fragment shader SPIR-V to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv"
	$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.comp.spv"
	$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Move image to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/resources/a.jpg"
	$<TARGET_FILE_DIR:${PROJECT_NAME}>
)