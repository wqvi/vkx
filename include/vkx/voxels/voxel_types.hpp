#pragma once

namespace vkx
{
  struct Voxel;
  struct VoxelMask;
  class VoxelMatrix;
  class GreedyMask;
  template <std::int32_t size>
  class VoxelChunk;
  
  enum class VoxelType : std::uint32_t
  {
    None,
    Air,
    Stone
  };
}