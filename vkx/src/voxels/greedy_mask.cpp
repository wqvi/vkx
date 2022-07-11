#include <vkx/voxels/greedy_mask.hpp>

#include <vkx/voxels/voxel_matrix.hpp>
#include <vkx/voxels/voxel_mask.hpp>

namespace vkx
{
  GreedyMask::GreedyMask(std::int32_t Axis1Limit, std::int32_t Axis2Limit)
      : Matrix2D<VoxelMask>(Axis1Limit, Axis2Limit) {}

  void GreedyMask::calculate(VoxelMatrix const &voxels, glm::i32vec3 &ChunkItr, std::int32_t Axis1, std::int32_t Axis2, glm::i32vec3 const &AxisMask)
  {
    for (ChunkItr[Axis2] = 0; ChunkItr[Axis2] < height; ++ChunkItr[Axis2])
    {
      for (ChunkItr[Axis1] = 0; ChunkItr[Axis1] < width; ++ChunkItr[Axis1])
      {
        const Voxel CurrentBlock = voxels.at(ChunkItr);
        // Checks the next block
        // Has potential to check out of bounds
        // Maybe add a feature to check into the next chunk?
        const Voxel CompareBlock = voxels.at(ChunkItr + AxisMask);

        const bool CurrentBlockOpaque = CurrentBlock.visible;
        const bool CompareBlockOpaque = CompareBlock.visible;

        if (CurrentBlockOpaque == CompareBlockOpaque)
          Matrix2D<VoxelMask>::set(ChunkItr[Axis1], ChunkItr[Axis2], VoxelMask{Voxel{VoxelType::None}, 0});
        else if (CurrentBlockOpaque)
          Matrix2D<VoxelMask>::set(ChunkItr[Axis1], ChunkItr[Axis2], VoxelMask{CurrentBlock, 1});
        else
          Matrix2D<VoxelMask>::set(ChunkItr[Axis1], ChunkItr[Axis2], VoxelMask{CompareBlock, -1});
      }
    }
  }

  void GreedyMask::clear(std::int32_t i, std::int32_t j, std::int32_t Width, std::int32_t Height)
  {
    for (int l = 0; l < Height; ++l)
    {
      for (int k = 0; k < Width; ++k)
      {
        Matrix2D<VoxelMask>::set(i + k, j + l, VoxelMask{Voxel{VoxelType::None}, 0});
      }
    }
  }

  std::int32_t GreedyMask::calculateQuadWidth(VoxelMask const &CurrentMask, std::int32_t i, std::int32_t j)
  {
    std::int32_t Width = 1;
		while (i + Width < width && (Matrix<VoxelMask>::at(index(i, j) + Width) == CurrentMask))
			Width++;
		return Width;
  }

  std::int32_t GreedyMask::calculateQuadHeight(VoxelMask const &CurrentMask, std::int32_t Width, std::int32_t i, std::int32_t j)
  {
    std::int32_t Height = 1;
		while (j + Height < height)
		{
			for (int k = 0; k < Width; ++k)
			{
				if (!(Matrix<VoxelMask>::at(index(i, j) + k + Height * width) == CurrentMask))
					return Height;
			}
			Height++;
		}
		return Height;
  }
}