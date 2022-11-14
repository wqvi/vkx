#pragma once

#include <vkx/renderer/core/renderer_types.hpp>

namespace vkx {
struct SwapchainInfo {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	explicit SwapchainInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	[[nodiscard]] VkSurfaceFormatKHR chooseSurfaceFormat() const;

	[[nodiscard]] VkPresentModeKHR choosePresentMode() const;

	[[nodiscard]] VkExtent2D chooseExtent(int width, int height) const;

	[[nodiscard]] std::uint32_t getImageCount() const;
    
	[[nodiscard]] bool isComplete() const;
};
} // namespace vkx