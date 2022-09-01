#include <vkx/renderer/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vkx::Image::Image(const std::string& file, const vkx::Allocator& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter) {
	int texWidth;
	int texHeight;
	int texChannels;
	auto* const pixels = stbi_load(file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image.");
	}

	const auto imageSize = static_cast<vk::DeviceSize>(texWidth * texHeight * STBI_rgb_alpha);

	const auto stagingResource = allocator.allocateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
	std::memcpy(stagingResource->allocationInfo.pMappedData, pixels, stagingResource->allocationInfo.size);
	resource = allocator.allocateImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	commandSubmitter->transitionImageLayout(resource->object, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	commandSubmitter->copyBufferToImage(stagingResource->object, resource->object, texWidth, texHeight);
	commandSubmitter->transitionImageLayout(resource->object, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	stbi_image_free(pixels);
}