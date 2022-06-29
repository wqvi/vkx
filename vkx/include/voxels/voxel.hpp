#pragma once

#include <voxels/voxel_types.hpp>

namespace vkx
{
  struct Voxel
  {
    VoxelType type = VoxelType::Air;
    bool visible = false;
    bool collision = false;

    constexpr Voxel() = default;

    constexpr explicit(false) Voxel(VoxelType type)
        : type(type)
    {
      switch (type)
      {
      case VoxelType::None:
        visible = false;
        collision = true;
        break;
      case VoxelType::Air:
        visible = false;
        break;
      default:
        visible = true;
        collision = true;
        break;
      }
    }

    constexpr Voxel(VoxelType type, bool visible)
        : type(type),
          visible(visible) {}

    auto operator<=>(Voxel const &other) const = default;
  };
}