#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Swapchain::Swapchain(vk::Device logicalDevice, VkRenderPass renderPass, VmaAllocator allocator, VkSwapchainKHR swapchain, VkExtent2D extent, VkFormat imageFormat, VkFormat depthFormat)
    : device(logicalDevice),
      allocator(allocator),
      swapchain(swapchain),
      imageExtent(extent) {
	const auto images = logicalDevice.getSwapchainImagesKHR(swapchain);

	for (const vk::Image image : images) {
		imageViews.push_back(vkx::createImageView(device, image, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT));
	}

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

		framebuffers[i] = vkx::create<VkFramebuffer>(
			vkCreateFramebuffer, 
			[](auto result) {
			    if (result != VK_SUCCESS) {
				    throw std::runtime_error("Failed to create swapchain framebuffer.");
				}
			}, logicalDevice, &framebufferInfo, nullptr);
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