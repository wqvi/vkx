#include <vkx/renderer/model.hpp>

vkx::Mesh::Mesh(const std::vector<vkx::Vertex>& vertices, const std::vector<std::uint32_t>& indices, VmaAllocator allocator) 
	: indexCount(indices.size()) {
	vertexAllocation = vkx::allocateBuffer(&vertexAllocationInfo, &vertexBuffer, allocator, vertices.data(), sizeof(vkx::Vertex) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	indexAllocation = vkx::allocateBuffer(&indexAllocationInfo, &indexBuffer, allocator, indices.data(), sizeof(std::uint32_t) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void vkx::Mesh::destroy(VmaAllocator allocator) const {
  vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
  vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
}

vkx::Texture::Texture(const char* file, VkDevice device, float maxAnisotropy, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter)
    : image(file, allocator, commandSubmitter),
      view(image.createTextureImageView(device)),
      sampler(vkx::createTextureSampler(device, maxAnisotropy)),
      info{sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      device(device) {}

VkDescriptorImageInfo vkx::Texture::createDescriptorImageInfo() const {
	return {sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
}

const VkDescriptorImageInfo* vkx::Texture::getInfo() const {
	return &info;
}

void vkx::Texture::destroy() const {
	image.destroy();
	vkDestroyImageView(device, view, nullptr);
	vkDestroySampler(device, sampler, nullptr);
}