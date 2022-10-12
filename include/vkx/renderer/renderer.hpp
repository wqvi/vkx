#pragma once

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

[[nodiscard]] inline auto findDepthFormat(vk::PhysicalDevice physicalDevice) {
    return findSupportedFormat(physicalDevice, vk::ImageTiling::eOptimal,vk::FormatFeatureFlagBits::eDepthStencilAttachment, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint});
}
} // namespace vkx
