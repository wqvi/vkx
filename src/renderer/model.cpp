#include "vkx/renderer/core/renderer_types.hpp"
#include <vkx/renderer/model.hpp>

vkx::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const vkx::Allocator& allocator)
    : vertex(allocator.allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator.allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(indices.size()) {}

vkx::Mesh::Mesh(const std::vector<vkx::Vertex>& vertices, const std::vector<std::uint32_t>& indices, VmaAllocator allocator)
    : vertexAllocation(vkx::allocateBuffer(&vertexAllocationInfo, &vertexBuffer, allocator, vertices.data(), vertices.size() * sizeof(vkx::Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)),
      indexAllocation(vkx::allocateBuffer(&indexAllocationInfo, &indexBuffer, allocator, indices.data(), indices.size() * sizeof(std::uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT)),
      indexCount(indices.size()) {}

vkx::Texture::Texture(const char* file, VkDevice device, float maxAnisotropy, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter)
    : image(file, allocator, commandSubmitter),
      view(vkx::createTextureImageView(device, image.resourceImage)),
      sampler(vkx::createTextureSampler(device, maxAnisotropy)),
      info(sampler, view, vk::ImageLayout::eShaderReadOnlyOptimal) {}

vk::DescriptorImageInfo vkx::Texture::createDescriptorImageInfo() const {
	return {sampler, view, vk::ImageLayout::eShaderReadOnlyOptimal};
}

const vk::DescriptorImageInfo* vkx::Texture::getInfo() const {
	return &info;
}

void vkx::Texture::destroy(VmaAllocator allocator, VkDevice device) const {
	image.destroy(allocator);
	vkDestroyImageView(device, view, nullptr);
	vkDestroySampler(device, sampler, nullptr);
}