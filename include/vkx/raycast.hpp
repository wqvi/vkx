#pragma once

#include <vkx/voxels/voxel_matrix.hpp>
#include <vkx/camera.hpp>

namespace vkx {
std::tuple<bool, glm::ivec3, glm::ivec3> raycast(const glm::vec3& chunkPosition, const Camera& camera, const VoxelMatrix& voxels);
}