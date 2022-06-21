#pragma once

#include <voxels/voxel_mask.hpp>
#include <util/multi_dimensional.hpp>

namespace vkx
{
  class GreedyMask : public Matrix2D<VoxelMask>
	{
	public:
		explicit GreedyMask(std::int32_t Axis1Limit, std::int32_t Axis2Limit);

		void calculate(VoxelMatrix const &voxels, glm::i32vec3 &ChunkItr, std::int32_t Axis1, std::int32_t Axis2, glm::i32vec3 const &AxisMask);

		void clear(std::int32_t i, std::int32_t j, std::int32_t Width, std::int32_t Height);

		[[nodiscard]] std::int32_t calculateQuadWidth(VoxelMask const &CurrentMask, std::int32_t i, std::int32_t j);

		[[nodiscard]] std::int32_t calculateQuadHeight(VoxelMask const &CurrentMask, std::int32_t Width, std::int32_t i, std::int32_t j);
	};
}