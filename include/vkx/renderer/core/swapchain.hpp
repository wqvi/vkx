#pragma once

#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/image.hpp>

namespace vkx {
class Swapchain {
private:
	friend class CommandSubmitter;

	vk::Device logicalDevice{};

	vk::UniqueSwapchainKHR swapchain{};
	vk::Extent2D imageExtent{};
	std::vector<vk::UniqueImageView> imageViews{};

	vkx::Image depthImage;
	vk::UniqueImageView depthImageView{};

	std::vector<vk::UniqueFramebuffer> framebuffers{};

public:
	Swapchain() = default;

	explicit Swapchain(const vkx::VulkanDevice& device, 
		const vkx::VulkanRenderPass& renderPass, 
		const vkx::VulkanAllocator& allocator,
		const vkx::SwapchainInfo& swapchainInfo,
		vk::UniqueSwapchainKHR&& uniqueSwapchain);

	inline VkFramebuffer operator[](std::size_t index) const noexcept {
		return *framebuffers[index];
	}

	vk::ResultValue<std::uint32_t> acquireNextImage(const vkx::SyncObjects& syncObjects) const;

	inline VkExtent2D extent() const noexcept {
		return static_cast<VkExtent2D>(imageExtent);
	}
};
} // namespace vkx