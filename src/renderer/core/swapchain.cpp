#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Swapchain::Swapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, VkSurfaceKHR surface, VmaAllocator allocator, SDL_Window* window) 
	: device(device), allocator(allocator) {
	const SwapchainInfo info{physicalDevice, surface};
	const QueueConfig config{physicalDevice, surface};

	swapchain = vkx::createSwapchain(device, surface, window, info, config);

	images = vkx::get<VkImage>("Failed to retrieve swapchain images", vkGetSwapchainImagesKHR, [](auto a) { return a != VK_SUCCESS; }, device, swapchain);
	
	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);

	const auto imageFormat = info.chooseSurfaceFormat().format;
	imageExtent = static_cast<VkExtent2D>(info.chooseExtent(width, height));

	for (const auto image : images) {
		imageViews.push_back(vkx::createImageView(device, image, static_cast<VkFormat>(imageFormat), VK_IMAGE_ASPECT_COLOR_BIT));
	}

	const auto depthFormat = vkx::findDepthFormat(physicalDevice);
	depthAllocation = vkx::allocateImage(nullptr, &depthImage, allocator, imageExtent.width, imageExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthImageView = vkx::createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	framebuffers.resize(imageViews.size());

	for (std::size_t i = 0; i < imageViews.size(); i++) {
		const std::array framebufferAttachments = {imageViews[i], depthImageView};

		const VkFramebufferCreateInfo framebufferInfo{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			renderPass,
			static_cast<std::uint32_t>(framebufferAttachments.size()),
			framebufferAttachments.data(),
			imageExtent.width,
			imageExtent.height,
			1};

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swapchain framebuffer");
		}
	}
}

VkResult vkx::Swapchain::acquireNextImage(VkDevice device, const vkx::SyncObjects& syncObjects, std::uint32_t* imageIndex) const {
	return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, syncObjects.imageAvailableSemaphore, {}, imageIndex);
}

void vkx::Swapchain::destroy() {
	vkDestroyImageView(device, depthImageView, nullptr);
	vmaDestroyImage(allocator, depthImage, depthAllocation);

	for (auto imageView : imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
}