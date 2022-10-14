#pragma once

#include "vkx/renderer/core/sync_objects.hpp"
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
class Swapchain {
private:
	friend class CommandSubmitter;

	vk::UniqueSwapchainKHR swapchain{};
	vk::Extent2D imageExtent{};
	std::vector<vk::Image> images{};
	std::vector<vk::UniqueImageView> imageViews{};

	std::shared_ptr<Allocation<vk::Image>> depthResource{};
	vk::UniqueImageView depthImageView{};

	std::vector<vk::UniqueFramebuffer> framebuffers{};

public:
	Swapchain() = default;

	explicit Swapchain(vk::Device device, vk::PhysicalDevice physicalDevice, vk::RenderPass renderPass, vk::SurfaceKHR surface, SDL_Window* const window, const vkx::Allocator& allocator);

	inline vk::Framebuffer operator[](std::size_t index) const noexcept {
		return *framebuffers[index];
	}

	vk::ResultValue<std::uint32_t> acquireNextImage(const vkx::Device& device, const vkx::SyncObjects& syncObjects) const;

	vk::ResultValue<std::uint32_t> acquireNextImage(vk::Device device, const vkx::SyncObjects& syncObject) const {
		std::uint32_t imageIndex = 0;
		const auto result = device.acquireNextImageKHR(*swapchain, UINT64_MAX, *syncObject.imageAvailableSemaphore, {}, &imageIndex);

		return {result, imageIndex};
	}

	inline vk::Extent2D extent() const noexcept {
		return imageExtent;
	}

private:
	static vk::UniqueSwapchainKHR createSwapchainUnique(vk::Device device, vk::SurfaceKHR surface, SDL_Window* window, const vkx::SwapchainInfo& info, const vkx::QueueConfig& config);
};
} // namespace vkx