#pragma once

#include "core/queue_config.hpp"
#include "core/swapchain_info.hpp"
#include "core/sync_objects.hpp"

namespace vkx {
template <class ArrayType, class Function, class Predicate, class... Parameters>
constexpr auto get(const char* errorMessage, Function function, Predicate predicate, Parameters... param) {
	std::uint32_t count = 0;
	auto result = function(param..., &count, nullptr);
	if (predicate(result)) {
		throw std::runtime_error(errorMessage);
	}

	std::vector<ArrayType> items{count};
	result = function(param..., &count, items.data());
	if (predicate(result)) {
		throw std::runtime_error(errorMessage);
	}

	return items;
}

template <class ObjectType, class Function, class Predicate, class... Parameters>
constexpr auto create(Function function, Predicate predicate, Parameters... param) {
	ObjectType object{};
	auto result = function(param..., &object);
	predicate(result);

	return object;
}

[[nodiscard]] VkInstance createInstance(SDL_Window* const window);

[[nodiscard]] VkSurfaceKHR createSurface(SDL_Window* const window, VkInstance instance);

[[nodiscard]] VkPhysicalDevice getBestPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

[[nodiscard]] VkDevice createDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice);

[[nodiscard]] VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, VkImageTiling tiling, VkFormatFeatureFlags features, const std::vector<VkFormat>& candidates);

[[nodiscard]] inline auto findDepthFormat(VkPhysicalDevice physicalDevice) {
	return findSupportedFormat(physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT});
}

[[nodiscard]] VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

[[nodiscard]] inline auto createTextureImageView(VkDevice device, VkImage image) {
	return createImageView(device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

[[nodiscard]] vk::UniqueSampler createTextureSamplerUnique(vk::Device device, float samplerAnisotropy);

[[nodiscard]] VkRenderPass createRenderPass(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Format format, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout, vk::AttachmentLoadOp loadOp);

[[nodiscard]] std::vector<vkx::SyncObjects> createSyncObjects(vk::Device device);

[[nodiscard]] VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, SDL_Window* window, const vkx::SwapchainInfo& info, const vkx::QueueConfig& config);

[[nodiscard]] VmaAllocator createAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);

[[nodiscard]] VmaAllocation allocateImage(VmaAllocationInfo* allocationInfo, VkImage* image, VmaAllocator allocator, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

[[nodiscard]] VmaAllocation allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);
} // namespace vkx
