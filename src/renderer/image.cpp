#include <vkx/renderer/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vkx/renderer/renderer.hpp>

vkx::Image::Image(const std::string& file, const vkx::Allocator& allocator, const vkx::CommandSubmitter& commandSubmitter) {
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
	// resource = allocator.allocateImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	// commandSubmitter.transitionImageLayout(resource->object, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	// commandSubmitter.copyBufferToImage(stagingResource->object, resource->object, texWidth, texHeight);
	// commandSubmitter.transitionImageLayout(resource->object, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	stbi_image_free(pixels);
}

vkx::Image::Image(const char* file, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter) {
	int texWidth;
	int texHeight;
	int texChannels;
	auto* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image.");
	}

	const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * static_cast<int>(STBI_rgb_alpha));

	// const auto stagingResource = allocator.allocateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	VmaAllocationInfo allocationInfo{};
	VkBuffer stagingBuffer = nullptr;
	const auto stagingAllocation = vkx::allocateBuffer(&allocationInfo, &stagingBuffer, allocator, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	// std::memcpy(stagingResource->allocationInfo.pMappedData, pixels, stagingResource->allocationInfo.size);

	std::memcpy(allocationInfo.pMappedData, pixels, allocationInfo.size);

	// resource = allocator.allocateImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	resourceAllocation = vkx::allocateImage(nullptr, &resourceImage, allocator, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	// commandSubmitter.transitionImageLayout(resource->object, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	commandSubmitter.transitionImageLayout(resourceImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	// commandSubmitter.copyBufferToImage(stagingResource->object, resource->object, texWidth, texHeight);

	commandSubmitter.copyBufferToImage(stagingBuffer, resourceImage, texWidth, texHeight);

	// commandSubmitter.transitionImageLayout(resource->object, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	commandSubmitter.transitionImageLayout(resourceImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

	stbi_image_free(pixels);
}

void vkx::Image::destroy(VmaAllocator allocator) const {
	vmaDestroyImage(allocator, resourceImage, resourceAllocation);
}