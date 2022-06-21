#include <renderer/model.hpp>

namespace vkx
{
  Mesh::Mesh(std::vector<Vertex> const &vertices, std::vector<std::uint32_t> const &indices, Device const &device)
    : vertexBuffer(vertices, device), indexBuffer(indices, device) {}

  Mesh::Mesh(std::size_t vertexCount, std::size_t indexCount, Device const &device)
    : vertexBuffer(vertexCount, vk::BufferUsageFlagBits::eVertexBuffer, device), indexBuffer(indexCount, vk::BufferUsageFlagBits::eIndexBuffer, device) {}
}