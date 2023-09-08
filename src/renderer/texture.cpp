#include <vkx/renderer/texture.hpp>
#include <vkx/renderer/commands.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

vkx::Texture::Texture(const std::string& file,
		      const vkx::VulkanDevice& device,
		      const vkx::VulkanAllocator& allocator,
		      const vkx::CommandSubmitter& commandSubmitter)
	: sampler(device.createTextureSampler()) {
	int width;
	int height;
	int channels;
	stbi_uc* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	auto size = static_cast<vk::DeviceSize>(width) * height * STBI_rgb_alpha;

	const auto staging = allocator.allocateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	staging.mapMemory(pixels);

	vk::Extent2D extent{static_cast<std::uint32_t>(width), 
		static_cast<std::uint32_t>(height)};

	image = allocator.allocateImage(extent, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

	commandSubmitter.transitionImageLayout(static_cast<vk::Image>(image), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	commandSubmitter.copyBufferToImage(static_cast<vk::Buffer>(staging), static_cast<vk::Image>(image), width, height);

	commandSubmitter.transitionImageLayout(static_cast<vk::Image>(image), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	stbi_image_free(pixels);
	staging.destroy();

	view = image.createView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

	descriptorImageInfo = {*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal};
}

const vk::DescriptorImageInfo* vkx::Texture::imageInfo() const noexcept {
	return &descriptorImageInfo;
}
