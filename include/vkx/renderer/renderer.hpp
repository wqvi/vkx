#pragma once

#include "core/queue_config.hpp"
#include "core/swapchain_info.hpp"
#include "core/sync_objects.hpp"

namespace vkx {
[[nodiscard]] VkInstance createInstance(SDL_Window* const window);

[[nodiscard]] VkSurfaceKHR createSurface(SDL_Window* const window, VkInstance instance);

[[nodiscard]] VkPhysicalDevice getBestPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

[[nodiscard]] VkDevice createDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice);

[[nodiscard]] VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, VkImageTiling tiling, VkFormatFeatureFlags features, const std::vector<VkFormat>& candidates);

[[nodiscard]] inline auto findDepthFormat(vk::PhysicalDevice physicalDevice) {
	return findSupportedFormat(physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT});
}

[[nodiscard]] vk::UniqueImageView createImageViewUnique(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

[[nodiscard]] inline vk::UniqueImageView createTextureImageViewUnique(vk::Device device, vk::Image image) {
	return createImageViewUnique(device, image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

[[nodiscard]] vk::UniqueSampler createTextureSamplerUnique(vk::Device device, float samplerAnisotropy);

[[nodiscard]] VkRenderPass createRenderPass(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Format format, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout, vk::AttachmentLoadOp loadOp);

[[nodiscard]] std::vector<vkx::SyncObjects> createSyncObjects(vk::Device device);

[[nodiscard]] vk::UniqueSwapchainKHR createSwapchainUnique(vk::Device device, vk::SurfaceKHR surface, SDL_Window* window, const vkx::SwapchainInfo& info, const vkx::QueueConfig& config);

[[nodiscard]] VmaAllocator createAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);
} // namespace vkx
