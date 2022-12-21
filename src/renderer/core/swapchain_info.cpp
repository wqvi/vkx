#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::SwapchainInfo::SwapchainInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	const auto predicate = [](VkResult result) {
		return result != VK_SUCCESS;
	};

	capabilities = vkx::getObject<VkSurfaceCapabilitiesKHR>(
	    "Failed to get physical device surface capabilites.",
	    vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
	    predicate,
	    physicalDevice, surface);

	const auto formats = vkx::getArray<VkSurfaceFormatKHR>(
	    "Failed to get physcial device surface formats.",
	    vkGetPhysicalDeviceSurfaceFormatsKHR,
	    predicate,
	    physicalDevice, surface);

	const auto presentModes = vkx::getArray<VkPresentModeKHR>(
	    "Failed to get physical device surface present modes.",
	    vkGetPhysicalDeviceSurfacePresentModesKHR,
	    predicate,
	    physicalDevice, surface);

	surfaceFormat = formats[0];
	for (const auto& format : formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surfaceFormat = format;
		}
	}

	for (const auto mode : presentModes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = mode;
		}
	}

	imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}
}

VkExtent2D vkx::SwapchainInfo::chooseExtent(std::uint32_t width, std::uint32_t height) const {
	if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	VkExtent2D extent{width, height};

	extent.width = glm::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = glm::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
}