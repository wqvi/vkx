#pragma once

#include "core/queue_config.hpp"
#include "core/swapchain_info.hpp"
#include "core/sync_objects.hpp"
#include <vector>

namespace vkx {
/**
 * @brief Create a Vulkan instance object
 *
 * @exception std::runtime_exception Gets thrown if debug info or instance can't be made
 * @param window The window to which is queried for extensions
 * @return vk::UniqueInstance
 */
[[nodiscard]] vk::UniqueInstance createInstance(SDL_Window* const window);

/**
 * @brief Create a surface object
 *
 * @exception std::runtime_exception Gets thrown if a surface can't be made
 * @param window The window to which to attach the Vulkan surface
 * @param instance The Vulkan instance handle
 * @return vk::UniqueSurfaceKHR
 */
[[nodiscard]] vk::UniqueSurfaceKHR createSurface(SDL_Window* const window, vk::Instance instance);

/**
 * @brief Get the best physical device for Vulkan usage.
 *
 * @exception std::runtime_exception Gets thrown if a suitable physical device can't be found
 * @param instance The Vulkan instance handle
 * @param surface The Vulkan surface handle
 * @return vk::PhysicalDevice
 */
[[nodiscard]] vk::PhysicalDevice getBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

/**
 * @brief Create a Vulkan logical device object.
 *
 * @exception std::runtime_exception Gets thrown if unable to create a logical device
 * @param instance The Vulkan instance handle
 * @param surface The Vulkan surface handle
 * @param physicalDevice The Vulkan physical device handle
 * @return vk::UniqueDevice
 */
[[nodiscard]] vk::UniqueDevice createDevice(vk::Instance instance, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice);

/**
 * @brief Finds a supported format based on provided parameters
 *
 * @param physicalDevice The Vulkan physical device handle
 * @param tiling The tiling flags used
 * @param features The feature flags used
 * @param candidates The candidate formats being compared to
 * @return vk::Format Returns undefined if candidates don't match specification, else it will return the valid candidate
 */
[[nodiscard]] vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice, vk::ImageTiling tiling, vk::FormatFeatureFlags features, const std::vector<vk::Format>& candidates);

/**
 * @brief Finds the supported depth format for the specified physical device
 *
 * @param physicalDevice The Vulkan physical device handle
 * @return vk::Format Returns undefined if candidates don't match specification, else it will return valid candidate
 */
[[nodiscard]] inline vk::Format findDepthFormat(vk::PhysicalDevice physicalDevice) {
	return findSupportedFormat(physicalDevice, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint});
}

/**
 * @brief Create a image view unique object
 *
 * @param device The Vulkan logical device handle
 * @param image The Vulkan image handle
 * @param format The format of the unique image view
 * @param aspectFlags The aspect flags of the unique image view
 * @return vk::UniqueImageView
 */
[[nodiscard]] vk::UniqueImageView createImageViewUnique(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

/**
 * @brief Create a texture image view unique object
 *
 * @param device The Vulkan logical device handle
 * @param image The Vulkan image handle
 * @return vk::UniqueImageView
 */
[[nodiscard]] inline vk::UniqueImageView createTextureImageViewUnique(vk::Device device, vk::Image image) {
	return createImageViewUnique(device, image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

/**
 * @brief Create a Texture Sampler Unique object
 *
 * @param device The Vulkan logical device handle
 * @param samplerAnisotropy The maximum sampler anisotropy for the sampler
 * @return vk::UniqueSampler
 */
[[nodiscard]] vk::UniqueSampler createTextureSamplerUnique(vk::Device device, float samplerAnisotropy);

/**
 * @brief Create a Render Pass Unique object
 *
 * @param device The Vulkan logical device handle
 * @param physicalDevice The Vulkan physical device handle
 * @param format The format that the Vulkan render pass will be
 * @param initialLayout The initial image layout of the render pass
 * @param finalLayout The final image layout of the render pass
 * @param loadOp The operation that will occur upon the load of the render pass
 * @return vk::UniqueRenderPass
 */
[[nodiscard]] vk::UniqueRenderPass createRenderPassUnique(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Format format, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout, vk::AttachmentLoadOp loadOp);

/**
 * @brief Create a Sync Objects object
 * 
 * @param device The Vulkan logical device handle 
 * @return std::vector<vkx::SyncObjects> 
 */
[[nodiscard]] std::vector<vkx::SyncObjects> createSyncObjects(vk::Device device);

/**
 * @brief Create a Swapchain Unique object
 * 
 * @param device The Vulkan logical device handle 
 * @param surface The Vulkan surface handle
 * @param window The SDL2 window handle
 * @param info The VKX Swapchain information
 * @param config The VKX physical device queue config
 * @return vk::UniqueSwapchainKHR 
 */
[[nodiscard]] vk::UniqueSwapchainKHR createSwapchainUnique(vk::Device device, vk::SurfaceKHR surface, SDL_Window* window, const vkx::SwapchainInfo& info, const vkx::QueueConfig& config);
} // namespace vkx
