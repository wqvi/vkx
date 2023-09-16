#pragma once

#include <vkx/renderer/queue_config.hpp>
#include <vkx/renderer/swapchain_info.hpp>
#include <vkx/renderer/sync_objects.hpp>
#include <vkx/renderer/image.hpp>

namespace vkx {
class Swapchain {
	friend class CommandSubmitter;

public:
	vk::Device logicalDevice{};
	vk::SwapchainKHR swapchain{};
	vk::Extent2D imageExtent{};
	std::vector<vk::ImageView> imageViews{};
	vkx::Image depthImage{};
	vk::ImageView depthImageView{};
	std::vector<vk::Framebuffer> framebuffers{};

	Swapchain() = default;

	explicit Swapchain(const vkx::VulkanInstance& instance,
			   vk::RenderPass renderPass,
			   const vkx::SwapchainInfo& swapchainInfo,
			   vk::SwapchainKHR swapchain);

	void destroy();

	vk::ResultValue<std::uint32_t> acquireNextImage(const vkx::SyncObjects& syncObjects) const;
};
} // namespace vkx
