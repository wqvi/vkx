#pragma once

#include <voxels/voxel_types.hpp>

namespace vkx
{
  struct Voxel
  {
    VoxelType type = VoxelType::Air;
    bool visible = false;

    constexpr Voxel() = default;

    constexpr explicit(false) Voxel(VoxelType type)
        : type(type)
    {
      switch (type)
      {
      case VoxelType::None:
        visible = false;
        break;
      case VoxelType::Air:
        visible = false;
        break;
      default:
        visible = true;
        break;
      }
    }

    constexpr Voxel(VoxelType type, bool visible)
        : type(type),
          visible(visible) {}

    auto operator<=>(Voxel const &other) const = default;
  };
}