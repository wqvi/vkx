#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Swapchain::Swapchain(const vkx::VulkanDevice& device, const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, vk::UniqueSwapchainKHR&& uniqueSwapchain, VkExtent2D extent, VkFormat imageFormat, VkFormat depthFormat)
    : logicalDevice(static_cast<VkDevice>(device)),
      allocator(allocator),
      swapchain(std::move(uniqueSwapchain)),
      imageExtent(extent) {
	const auto images = logicalDevice.getSwapchainImagesKHR(*swapchain);

	imageViews.reserve(images.size());
	for (const vk::Image image : images) {
		imageViews.emplace_back(device.createImageView(image, static_cast<vk::Format>(imageFormat), vk::ImageAspectFlagBits::eColor));
	}

	depthImage = allocator.allocateImage(imageExtent, static_cast<vk::Format>(depthFormat), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageView = depthImage.createView(static_cast<vk::Format>(depthFormat), vk::ImageAspectFlagBits::eDepth);
	framebuffers.reserve(imageViews.size());

	for (std::size_t i = 0; i < imageViews.size(); i++) {
		const std::array framebufferAttachments = {*imageViews[i], *depthImageView};

		const vk::FramebufferCreateInfo framebufferCreateInfo{
		    {},
		    static_cast<VkRenderPass>(renderPass),
		    framebufferAttachments,
		    imageExtent.width,
		    imageExtent.height,
		    1};

		framebuffers.emplace_back(logicalDevice.createFramebufferUnique(framebufferCreateInfo));
	}
}

VkResult vkx::Swapchain::acquireNextImage(VkDevice device, const vkx::SyncObjects& syncObjects, std::uint32_t* imageIndex) const {
	return vkAcquireNextImageKHR(device, *swapchain, UINT64_MAX, syncObjects.imageAvailableSemaphore, {}, imageIndex);
}