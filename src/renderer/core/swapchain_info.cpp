#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::SwapchainInfo::SwapchainInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities) != VK_SUCCESS) {
		throw std::runtime_error("Failed to get physical device surface capabilities.");
	}

	formats = vkx::getArray<VkSurfaceFormatKHR>(
		"Failed to get physcial device surface formats.",
		vkGetPhysicalDeviceSurfaceFormatsKHR,
		[](auto a) { return a != VK_SUCCESS; },
		physicalDevice, surface);

	presentModes = vkx::getArray<VkPresentModeKHR>(
	    "Failed to get physical device surface present modes.",
	    vkGetPhysicalDeviceSurfacePresentModesKHR,
	    [](auto a) { return a != VK_SUCCESS; },
	    physicalDevice, surface);
}

VkSurfaceFormatKHR vkx::SwapchainInfo::chooseSurfaceFormat() const {
	for (const auto& surfaceFormat : formats) {
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return surfaceFormat;
		}
	}

	return formats[0];
}

VkPresentModeKHR vkx::SwapchainInfo::choosePresentMode() const {
	for (const auto presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vkx::SwapchainInfo::chooseExtent(int width, int height) const {
	if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	VkExtent2D extent{
	    static_cast<std::uint32_t>(width),
	    static_cast<std::uint32_t>(height)};

	extent.width = glm::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = glm::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
}

std::uint32_t vkx::SwapchainInfo::getImageCount() const {
	std::uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		return capabilities.maxImageCount;
	}
	return imageCount;
}

bool vkx::SwapchainInfo::isComplete() const {
	return !formats.empty() && !presentModes.empty();
}