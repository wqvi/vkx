#pragma once

namespace vkx
{
  struct VoxelMask;
  class VoxelMatrix;
  class GreedyMask;
  template <std::int32_t size>
  class VoxelChunk;
  
  enum class Voxel : std::uint32_t
  {
    Air,
    Stone
  };
}