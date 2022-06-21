#include <voxels/voxel_matrix.hpp>
#include <voxels/voxel.hpp>

namespace vkx
{
  Voxel const &VoxelMatrix::at(glm::i32vec3 const &location) const
  {
    if (!validLocation(location.x, location.y, location.z))
    {
      static Voxel air{VoxelType::Air, false};
      return air;
    }
    return Matrix3D<Voxel>::at(location.x, location.y, location.z);
  }
}