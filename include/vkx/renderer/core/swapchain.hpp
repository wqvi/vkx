#pragma once

#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/core/device.hpp>

namespace vkx {
class Swapchain {
public:
	Swapchain() = default;

	Swapchain(const Device& device, vk::SurfaceKHR surface, SDL_Window* window, const std::shared_ptr<Allocator>& allocator);

	operator const vk::SwapchainKHR&() const;

	operator const vk::UniqueSwapchainKHR&() const;

	void createFramebuffers(vk::Device device, vk::RenderPass renderPass);

	vk::ResultValue<std::uint32_t> acquireNextImage(vk::Device device, const vkx::SyncObjects& syncObjects) const;

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