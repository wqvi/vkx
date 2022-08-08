#pragma once

#include <vkx/renderer/core/renderer_base.hpp>
#include <vkx/voxels/greedy_mask.hpp>
#include <vkx/voxels/voxel_matrix.hpp>

namespace vkx {
class VoxelChunk {
public:
	explicit VoxelChunk(const glm::vec3& worldPosition, std::int32_t size);

	void greedy();

	std::vector<vkx::Vertex> vertices;
	std::vector<std::uint32_t> indices;

private:
	void generateMesh(GreedyMask& Mask, std::int32_t Axis1, std::int32_t Axis2, const glm::i32vec3& AxisMask, glm::i32vec3& ChunkItr);

	void createQuad(
	    VoxelMask mask,
	    const glm::ivec3& axisMask,
	    const int width,
	    const int height,
	    const glm::ivec3& V1,
	    const glm::ivec3& V2,
	    const glm::ivec3& V3,
	    const glm::ivec3& V4);

	const std::int32_t axisLimit = 16;
	VoxelMatrix voxels;
	std::uint32_t vertexCount = 0;
	glm::vec3 worldPosition = glm::vec3(0);
};
} // namespace vkx