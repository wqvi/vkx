#include <vkx/renderer/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vkx::Image::Image(const char* file, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter) 
	: allocator(allocator) {
	int texWidth;
	int texHeight;
	int texChannels;
	auto* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image.");
	}

	const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * static_cast<int>(STBI_rgb_alpha));

	VmaAllocationInfo allocationInfo{};
	VkBuffer stagingBuffer = nullptr;
	const auto stagingAllocation = vkx::allocateBuffer(&allocationInfo, &stagingBuffer, allocator, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	std::memcpy(allocationInfo.pMappedData, pixels, allocationInfo.size);

	resourceAllocation = vkx::allocateImage(nullptr, &resourceImage, allocator, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	commandSubmitter.transitionImageLayout(resourceImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	commandSubmitter.copyBufferToImage(stagingBuffer, resourceImage, texWidth, texHeight);

	commandSubmitter.transitionImageLayout(resourceImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

	stbi_image_free(pixels);
}

void vkx::Image::destroy() const {
	vmaDestroyImage(allocator, resourceImage, resourceAllocation);
}