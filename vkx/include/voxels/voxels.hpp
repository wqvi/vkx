#pragma once

#include <renderer/core/renderer_base.hpp>
#include <voxels/voxel_matrix.hpp>
#include <voxels/greedy_mask.hpp>

namespace vkx
{
	class VoxelChunk
	{
	public:
		explicit VoxelChunk(std::int32_t width, std::int32_t height, std::int32_t depth);

		explicit VoxelChunk(std::int32_t size);

		void greedy();

		std::vector<vkx::Vertex> vertices;
		std::vector<std::uint32_t> indices;

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
	};
}