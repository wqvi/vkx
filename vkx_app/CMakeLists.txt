cmake_minimum_required(VERSION 3.0.0)
project(voxel_app
        LANGUAGES CXX
        VERSION 0.0.0)

add_executable(voxel_app "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

target_link_libraries(voxel_app PUBLIC VoxelEngine)