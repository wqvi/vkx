#include "vkx/renderer/core/device.hpp"
#include <cstring>
#include <stdexcept>
#include <vk_mem_alloc.h>
#include <vkx/renderer/image.hpp>
#include <vulkan/vulkan_enums.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vkx::Image::Image(const std::string& file, const Device& device, const std::shared_ptr<vkx::Allocator>& allocator) {
	int texWidth;
	int texHeight;
	int texChannels;
	auto* pixels = stbi_load(file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image.");
	}

	const auto imageSize = static_cast<vk::DeviceSize>(texWidth * texHeight * STBI_rgb_alpha);

	const auto stagingResource = allocator->allocateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
	std::memcpy(stagingResource->allocationInfo.pMappedData, pixels, stagingResource->allocationInfo.size);
	resource = allocator->allocateImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	device.transitionImageLayout(resource->object, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	device.copyBufferToImage(stagingResource->object, resource->object, texWidth, texHeight);
	device.transitionImageLayout(resource->object, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	stbi_image_free(pixels);
}