#pragma once

#include <vkx/renderer/core/renderer_types.hpp>

namespace vkx {
struct SwapchainInfo {
	vk::SurfaceTransformFlagBitsKHR currentTransform{};
	vk::Format surfaceFormat{};
	vk::ColorSpaceKHR surfaceColorSpace{};
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	std::uint32_t imageCount = 0;
	vk::Extent2D actualExtent{};

	explicit SwapchainInfo(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const vkx::Window& window);
};
} // namespace vkx