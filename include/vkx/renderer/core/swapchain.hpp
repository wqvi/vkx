#pragma once

#include "vkx/renderer/core/sync_objects.hpp"
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>

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

	explicit Swapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, VkSurfaceKHR surface, VmaAllocator allocator, SDL_Window* window);

	inline VkFramebuffer operator[](std::size_t index) const noexcept {
		return framebuffers[index];
	}

	VkResult acquireNextImage(vk::Device device, const vkx::SyncObjects& syncObjects, std::uint32_t* imageIndex) const;

	inline VkExtent2D extent() const noexcept {
		return imageExtent;
	}

	void destroy();
};
} // namespace vkx