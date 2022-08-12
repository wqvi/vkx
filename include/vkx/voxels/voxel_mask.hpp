#pragma once

#include "voxel_types.hpp"
#include "voxel.hpp"

namespace vkx
{
  struct VoxelMask
  {
    Voxel voxel = VoxelType::None;
    std::int32_t normal = 0;

    constexpr VoxelMask() = default;

    constexpr VoxelMask(Voxel const &voxel, std::int32_t normal)
        : voxel(voxel),
          normal(normal) {}

    // auto operator<=>(const VoxelMask &other) const = default;

    bool operator==(const VoxelMask &other) const {
      return voxel == other.voxel && normal == other.normal;
    }

    bool operator!=(const VoxelMask &other) const {
      return voxel != other.voxel && normal != other.normal;
    }
  };
}