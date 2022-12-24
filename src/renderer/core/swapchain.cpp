#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Swapchain::Swapchain(vk::Device logicalDevice, VkRenderPass renderPass, const vkx::VulkanAllocator& allocator, vk::UniqueSwapchainKHR&& uniqueSwapchain, VkExtent2D extent, VkFormat imageFormat, VkFormat depthFormat)
    : device(logicalDevice),
      allocator(allocator),
      swapchain(std::move(uniqueSwapchain)),
      imageExtent(extent) {
	const auto images = logicalDevice.getSwapchainImagesKHR(*swapchain);

	imageViews.reserve(images.size());
	for (const vk::Image image : images) {
		imageViews.emplace_back(vkx::createImageView(device, image, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT));
	}

	depthImage = allocator.allocateImage(static_cast<vk::Extent2D>(imageExtent), static_cast<vk::Format>(depthFormat), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageView = depthImage.createView(static_cast<vk::Format>(depthFormat), vk::ImageAspectFlagBits::eDepth);
	framebuffers.reserve(imageViews.size());

	for (std::size_t i = 0; i < imageViews.size(); i++) {
		const std::array framebufferAttachments = {imageViews[i], *depthImageView};

		const vk::FramebufferCreateInfo framebufferCreateInfo{
		    {},
		    renderPass,
		    framebufferAttachments,
		    imageExtent.width,
		    imageExtent.height,
		    1};

		framebuffers.emplace_back(logicalDevice.createFramebuffer(framebufferCreateInfo));
	}
}

VkResult vkx::Swapchain::acquireNextImage(VkDevice device, const vkx::SyncObjects& syncObjects, std::uint32_t* imageIndex) const {
	return vkAcquireNextImageKHR(device, *swapchain, UINT64_MAX, syncObjects.imageAvailableSemaphore, {}, imageIndex);
}

void vkx::Swapchain::destroy() {
	//vkDestroyImageView(device, depthImageView, nullptr);
	//vmaDestroyImage(allocator, depthImage, depthAllocation);

	for (auto imageView : imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	//for (auto framebuffer : framebuffers) {
		//vkDestroyFramebuffer(device, framebuffer, nullptr);
	//}

	//vkDestroySwapchainKHR(device, swapchain, nullptr);
}