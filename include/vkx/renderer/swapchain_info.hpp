#pragma once

#include <vkx/window.hpp>

namespace vkx {
struct SwapchainInfo {
	vk::SurfaceTransformFlagBitsKHR currentTransform{};
	vk::Format surfaceFormat{};
	vk::ColorSpaceKHR surfaceColorSpace{};
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	std::uint32_t imageCount = 0;
	vk::Extent2D actualExtent{};

	explicit SwapchainInfo(vk::PhysicalDevice physicalDevice,
			       vk::SurfaceKHR surface,
			       SDL_Window* window);
};
} // namespace vkx
