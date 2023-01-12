#include <vkx/renderer/model.hpp>

vkx::Mesh::Mesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanAllocator& allocator)
    : vertexBuffer(allocator.allocateBuffer(vertices.data(), vertices.size() * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(allocator.allocateBuffer(indices.data(), indices.size() * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(std::move(vertices)),
      indices(std::move(indices)),
      activeIndexCount(activeIndexCount) {
}

vkx::Mesh::Mesh(std::size_t vertexCount, std::size_t indexCount, const vkx::VulkanAllocator& allocator)
    : vertexBuffer(allocator.allocateBuffer(vertexCount * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(allocator.allocateBuffer(indexCount * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(vertexCount),
      indices(indexCount) {
}

vkx::ArrayMesh::ArrayMesh(const vkx::VulkanAllocator& allocator)
    : vertexPool(allocator.allocateBufferPool(vk::BufferUsageFlagBits::eVertexBuffer, 0, 0)),
      vertexBuffers(allocator.allocateBuffers(0, 0)),
      indexPool(allocator.allocateBufferPool(vk::BufferUsageFlagBits::eIndexBuffer, 0, 0)) {

	throw std::runtime_error("Not implemented.");
}
