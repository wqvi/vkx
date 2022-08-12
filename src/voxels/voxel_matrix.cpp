#include <vkx/voxels/voxel_matrix.hpp>
#include <vkx/voxels/voxel.hpp>

namespace vkx
{
  Voxel VoxelMatrix::at(glm::i32vec3 const &location) const
  {
    if (!validLocation(location.x, location.y, location.z))
    {
      return Voxel::Air;
    }
    return Matrix3D<Voxel>::at(location.x, location.y, location.z);
  }
}