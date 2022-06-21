#pragma once

#include <voxels/voxel.hpp>
#include <util/multi_dimensional.hpp>

namespace vkx
{
  class VoxelMatrix final : public Matrix3D<Voxel>
  {
  public:
    using Matrix3D<Voxel>::Matrix3D;

    ~VoxelMatrix() final = default;

    [[nodiscard]] Voxel const &at(glm::i32vec3 const &location) const;
  };
}