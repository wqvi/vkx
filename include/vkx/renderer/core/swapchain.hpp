#pragma once

#include <vkx/renderer/core/device.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
class Swapchain {
public:
	Swapchain();

	Swapchain(const Device& device, const vk::UniqueSurfaceKHR& surface, SDL_Window* window, const std::shared_ptr<Allocator>& allocator);

	operator const vk::SwapchainKHR&() const;

	operator const vk::UniqueSwapchainKHR&() const;

	void createFramebuffers(const Device& device, const vk::UniqueRenderPass& renderPass);

	vk::ResultValue<std::uint32_t> acquireNextImage(const Device& device, const vk::UniqueSemaphore& semaphore) const;

	vk::UniqueSwapchainKHR swapchain;
	vk::Format imageFormat;
	vk::Extent2D extent;
	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;

	std::shared_ptr<Allocation<vk::Image>> depthResource;
	vk::UniqueImageView depthImageView;

	std::vector<vk::UniqueFramebuffer> framebuffers;
};
} // namespace vkx