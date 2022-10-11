#pragma once

#include <vkx/application.hpp>
#include <vkx/renderer/core/bootstrap.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/model.hpp>
#include <vulkan/vulkan_handles.hpp>

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

template <class T>
[[nodiscard]] vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice, vk::ImageTiling tiling, vk::FormatFeatureFlags features, const T& candidates) {
	for (const vk::Format format : candidates) {
		const auto formatProps = physicalDevice.getFormatProperties(format);

		const bool isLinear = tiling == vk::ImageTiling::eLinear && (formatProps.linearTilingFeatures & features) == features;
		const bool isOptimal = tiling == vk::ImageTiling::eOptimal && (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return vk::Format::eUndefined;
}
} // namespace vkx
