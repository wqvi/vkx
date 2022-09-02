#pragma once

#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>

namespace vkx {
class Swapchain {
private:
	friend class CommandSubmitter;

	vk::UniqueSwapchainKHR swapchain{};
	vk::Format imageFormat{};
	vk::Extent2D imageExtent{};
	std::vector<vk::Image> images{};
	std::vector<vk::UniqueImageView> imageViews{};

	std::shared_ptr<Allocation<vk::Image>> depthResource{};
	vk::UniqueImageView depthImageView{};

	std::vector<vk::UniqueFramebuffer> framebuffers{};

public:
	Swapchain() = default;

	explicit Swapchain(const Device& device, vk::SurfaceKHR surface, SDL_Window* window, const Allocator& allocator);

	explicit Swapchain(const Device& device, vk::RenderPass renderPass, vk::SurfaceKHR surface, SDL_Window* window, const Allocator& allocator);

	inline vk::Framebuffer operator[](std::size_t index) const noexcept {
		return *framebuffers[index];
	}

	void createFramebuffers(const Device& device, vk::RenderPass renderPass);

	vk::ResultValue<std::uint32_t> acquireNextImage(const vkx::Device& device, const vkx::SyncObjects& syncObjects) const;

	inline vk::Format format() const noexcept {
		return imageFormat;
	}

	inline vk::Extent2D extent() const noexcept {
		return imageExtent;
	}

private:
	static vk::UniqueSwapchainKHR createSwapchain(const Device& device, const SwapchainInfo& info, const QueueConfig& config, vk::SurfaceKHR surface, SDL_Window* window);
};
} // namespace vkx