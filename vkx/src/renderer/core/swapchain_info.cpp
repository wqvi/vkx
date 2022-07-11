#include <vkx/renderer/core/swapchain_info.hpp>

namespace vkx
{
	SwapchainInfo::SwapchainInfo(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface)
			: capabilities(physicalDevice.getSurfaceCapabilitiesKHR(*surface)),
				formats(physicalDevice.getSurfaceFormatsKHR(*surface)),
				presentModes(physicalDevice.getSurfacePresentModesKHR(*surface)) {}

	vk::SurfaceFormatKHR SwapchainInfo::chooseSurfaceFormat() const
	{
		for (auto const &surfaceFormat : formats)
		{
			if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return surfaceFormat;
			}
		}

		return formats.at(0);
	}

	vk::PresentModeKHR SwapchainInfo::choosePresentMode() const
	{
		for (auto const &presentMode : presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				return presentMode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D SwapchainInfo::chooseExtent(int width, int height) const
	{
		if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		vk::Extent2D extent{
				static_cast<std::uint32_t>(width), // width
				static_cast<std::uint32_t>(height) // height
		};

		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}

	std::uint32_t SwapchainInfo::getImageCount() const
	{
		std::uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			return capabilities.maxImageCount;
		}
		return imageCount;
	}

	bool SwapchainInfo::isComplete() const
	{
		return !formats.empty() && !presentModes.empty();
	}
}