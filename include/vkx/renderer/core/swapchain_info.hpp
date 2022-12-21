#pragma once

#include <vkx/renderer/core/renderer_types.hpp>

namespace vkx {
struct SwapchainInfo {
	VkSurfaceCapabilitiesKHR capabilities{};
	VkSurfaceFormatKHR surfaceFormat{};
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	std::uint32_t imageCount = 0;

	explicit SwapchainInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	[[nodiscard]] VkExtent2D chooseExtent(std::uint32_t width, std::uint32_t height) const;
};
} // namespace vkx