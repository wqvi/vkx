#pragma once

#include <voxels/voxel_types.hpp>
#include <voxels/voxel.hpp>

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

    auto operator<=>(const VoxelMask &other) const = default;
  };
}