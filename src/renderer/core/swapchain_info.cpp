#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>
#include <vkx/window.hpp>

vkx::SwapchainInfo::SwapchainInfo(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const vkx::Window& window) {
	const auto formats = physicalDevice.getSurfaceFormatsKHR(surface);

	surfaceFormat = formats[0].format;
	surfaceColorSpace = formats[0].colorSpace;
	for (auto format : formats) {
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			surfaceFormat = format.format;
			surfaceColorSpace = format.colorSpace;
		}
	}

	const auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	for (const auto mode : presentModes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			presentMode = mode;
		}
	}

	const auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

	imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	currentTransform = capabilities.currentTransform;

	if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
		actualExtent = capabilities.currentExtent;
	} else {
		const auto [width, height] = window.getDimensions();

		actualExtent = vk::Extent2D{static_cast<std::uint32_t>(width),
					    static_cast<std::uint32_t>(height)};

		actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}
}