#include <vkx/renderer/model.hpp>

vkx::Texture::Texture(const std::string& file, const vkx::VulkanDevice& device, const vkx::VulkanAllocator& allocator, const vkx::CommandSubmitter& commandSubmitter)
    : image(allocator.allocateImage(commandSubmitter, file, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)),
      view(image.createView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor)),
      sampler(device.createTextureSampler()),
      info(*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal) {}

vk::DescriptorImageInfo vkx::Texture::createDescriptorImageInfo() const {
	return {*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal};
}

const vk::DescriptorImageInfo* vkx::Texture::getInfo() const {
	return &info;
}
