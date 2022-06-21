cmake_minimum_required(VERSION 3.0.0)
project(VoxelApp
        LANGUAGES CXX
        VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(VoxelApp "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

target_link_libraries(VoxelApp PUBLIC vkx)