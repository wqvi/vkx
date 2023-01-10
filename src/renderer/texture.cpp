#include <vkx/renderer/texture.hpp>

vkx::Texture::Texture(const std::string& file,
		      const vkx::VulkanDevice& device,
		      const vkx::VulkanAllocator& allocator,
		      const vkx::CommandSubmitter& commandSubmitter)
    : image(allocator.allocateImage(commandSubmitter, file, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)),
      view(image.createView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor)),
      sampler(device.createTextureSampler()),
      descriptorImageInfo{*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal} {}

const vk::DescriptorImageInfo* vkx::Texture::imageInfo() const noexcept {
	return &descriptorImageInfo;
}