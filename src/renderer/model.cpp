#include <vkx/renderer/model.hpp>

vkx::Mesh::Mesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanAllocator& allocator)
    : vertexBuffer(allocator.allocateBuffer(vertices.size() * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(allocator.allocateBuffer(indices.size() * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(std::move(vertices)),
      indices(std::move(indices)),
      activeIndexCount(activeIndexCount) {
	vertexBuffer.mapMemory(vertices.data());
	indexBuffer.mapMemory(indices.data());
}

vkx::Mesh::Mesh(std::size_t vertexCount, std::size_t indexCount, const vkx::VulkanAllocator& allocator)
    : vertexBuffer(allocator.allocateBuffer(vertexCount * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(allocator.allocateBuffer(indexCount * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(vertexCount),
      indices(indexCount) {
}

vkx::ArrayMesh::ArrayMesh(std::size_t vertexBlockSize,
			  std::size_t vertexBlockCount,
			  std::size_t indexBlockSize,
			  std::size_t indexBlockCount,
			  const vkx::VulkanAllocator& allocator)
    : vertexPool(allocator.allocateBufferPool(vk::BufferUsageFlagBits::eVertexBuffer, vertexBlockSize, vertexBlockCount)),
      vertexBuffers(vertexPool.allocateBuffers()),
      indexBuffer(allocator.allocateBuffer(indexBlockSize * indexBlockCount, vk::BufferUsageFlagBits::eIndexBuffer)) {
}

void vkx::ArrayMesh::draw(const vkx::CommandSubmitter& commandSubmitter) const {
}
