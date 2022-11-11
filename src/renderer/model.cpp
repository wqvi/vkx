#include <vkx/renderer/model.hpp>

void vkx::Mesh::destroy(VmaAllocator allocator) const {
  vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
  vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
}

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