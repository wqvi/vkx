#pragma once

#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Swapchain {
private:
	friend class CommandSubmitter;

	vk::Device logicalDevice{};
	VmaAllocator allocator{};

	vk::UniqueSwapchainKHR swapchain{};
	vk::Extent2D imageExtent{};
	std::vector<vk::UniqueImageView> imageViews{};

	vkx::Image depthImage;
	vk::UniqueImageView depthImageView{};

	std::vector<vk::UniqueFramebuffer> framebuffers{};

public:
	Swapchain() = default;

	explicit Swapchain(const vkx::VulkanDevice& device, const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, vk::UniqueSwapchainKHR&& uniqueSwapchain, VkExtent2D extent, VkFormat imageFormat, VkFormat depthFormat);

	inline VkFramebuffer operator[](std::size_t index) const noexcept {
		return *framebuffers[index];
	}

	VkResult acquireNextImage(VkDevice device, const vkx::SyncObjects& syncObjects, std::uint32_t* imageIndex) const;

	inline VkExtent2D extent() const noexcept {
		return imageExtent;
	}
};
} // namespace vkx