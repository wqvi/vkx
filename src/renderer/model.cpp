#include <vkx/renderer/model.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Mesh::Mesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanInstance& instance)
    : vertexBuffer(instance.allocateBuffer(vertices.size() * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(instance.allocateBuffer(indices.size() * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(std::move(vertices)),
      indices(std::move(indices)),
      activeIndexCount(activeIndexCount) {
	vertexBuffer.mapMemory(vertices.data());
	indexBuffer.mapMemory(indices.data());
}

vkx::Mesh::Mesh(std::size_t vertexCount, std::size_t indexCount, const vkx::VulkanInstance& instance)
    : vertexBuffer(instance.allocateBuffer(vertexCount * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(instance.allocateBuffer(indexCount * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(vertexCount),
      indices(indexCount) {
}
