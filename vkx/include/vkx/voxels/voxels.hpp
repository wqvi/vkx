#pragma once

#include <vkx/renderer/core/renderer_base.hpp>
#include <vkx/voxels/voxel_matrix.hpp>
#include <vkx/voxels/greedy_mask.hpp>

namespace vkx
{
	class VoxelChunk
	{
	public:
		explicit VoxelChunk(const glm::vec3 &worldPosition, std::int32_t width, std::int32_t height, std::int32_t depth);

		explicit VoxelChunk(const glm::vec3 &worldPosition, std::int32_t size);

		void greedy();

		std::vector<vkx::Vertex> vertices;
		std::vector<std::uint32_t> indices;

        glm::vec3 test(const glm::vec3 &position);

	private:
		void generateMesh(GreedyMask &Mask, std::int32_t Axis1, std::int32_t Axis2, glm::i32vec3 const &AxisMask, glm::i32vec3 &ChunkItr);

		void CreateQuad(
				VoxelMask mask,
				const glm::ivec3 &axisMask,
				const int width,
				const int height,
				const glm::ivec3 &V1,
				const glm::ivec3 &V2,
				const glm::ivec3 &V3,
				const glm::ivec3 &V4);

		VoxelMatrix voxels;
		std::uint32_t vertexCount = 0;
        glm::vec3 worldPosition = glm::vec3(0);
	};
}