#include <vkx/renderer/model.hpp>

vkx::Texture::Texture(const char* file, VkDevice device, float maxAnisotropy, const vkx::VulkanAllocator& allocator, const vkx::CommandSubmitter& commandSubmitter)
    : image(file, {}, {}, allocator, commandSubmitter),
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
