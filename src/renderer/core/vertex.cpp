#include <vkx/renderer/core/vertex.hpp>

vkx::VoxelVertex::VoxelVertex(const glm::vec2& pos, const glm::vec2& uv, const glm::vec2& normal)
    : pos(pos), uv(uv), normal(normal) {}