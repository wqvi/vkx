#pragma once

#include <vkx/renderer/queue_config.hpp>
#include <vkx/renderer/swapchain_info.hpp>
#include <vkx/renderer/sync_objects.hpp>
#include <vkx/renderer/image.hpp>

namespace vkx {
class Swapchain {
	friend class CommandSubmitter;

private:
	vk::Device logicalDevice{};
	vk::UniqueSwapchainKHR swapchain{};
	vk::Extent2D imageExtent{};
	std::vector<vk::UniqueImageView> imageViews{};
	vkx::Image depthImage{};
	vk::UniqueImageView depthImageView{};
	std::vector<vk::UniqueFramebuffer> framebuffers{};

public:
	Swapchain() = default;

	explicit Swapchain(const vkx::VulkanDevice& device,
			   const vkx::VulkanRenderPass& renderPass,
			   const vkx::VulkanAllocator& allocator,
			   const vkx::SwapchainInfo& swapchainInfo,
			   vk::UniqueSwapchainKHR&& uniqueSwapchain);

	vk::ResultValue<std::uint32_t> acquireNextImage(const vkx::SyncObjects& syncObjects) const;
};
} // namespace vkx