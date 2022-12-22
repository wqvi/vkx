#pragma once

#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Swapchain {
private:
	friend class CommandSubmitter;

	VkDevice device{};
	VmaAllocator allocator{};

	VkSwapchainKHR swapchain{};
	VkExtent2D imageExtent{};
	std::vector<VkImage> images{};
	std::vector<VkImageView> imageViews{};

	VkImage depthImage{};
	VmaAllocation depthAllocation{};
	VkImageView depthImageView{};

	std::vector<VkFramebuffer> framebuffers{};

public:
	Swapchain() = default;

	explicit Swapchain(VkDevice logicalDevice, VkRenderPass renderPass, VmaAllocator allocator, VkSwapchainKHR swapchain, VkExtent2D extent, VkFormat imageFormat, VkFormat depthFormat);

	inline VkFramebuffer operator[](std::size_t index) const noexcept {
		return framebuffers[index];
	}

	VkResult acquireNextImage(VkDevice device, const vkx::SyncObjects& syncObjects, std::uint32_t* imageIndex) const;

	inline VkExtent2D extent() const noexcept {
		return imageExtent;
	}

	void destroy();
};
} // namespace vkx