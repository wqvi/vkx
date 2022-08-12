#include <vkx/renderer/core/swapchain_info.hpp>

vkx::SwapchainInfo::SwapchainInfo(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
    : capabilities(physicalDevice.getSurfaceCapabilitiesKHR(surface)),
      formats(physicalDevice.getSurfaceFormatsKHR(surface)),
      presentModes(physicalDevice.getSurfacePresentModesKHR(surface)) {}

vk::SurfaceFormatKHR vkx::SwapchainInfo::chooseSurfaceFormat() const {
	for (const auto& surfaceFormat : formats) {
		if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return surfaceFormat;
		}
	}

	return formats[0];
}

vk::PresentModeKHR vkx::SwapchainInfo::choosePresentMode() const {
	for (const auto presentMode : presentModes) {
		if (presentMode == vk::PresentModeKHR::eMailbox) {
			return presentMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D vkx::SwapchainInfo::chooseExtent(int width, int height) const {
	if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	vk::Extent2D extent(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));

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