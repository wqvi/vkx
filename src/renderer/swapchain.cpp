#include <vkx/renderer/swapchain.hpp>

vkx::Swapchain::Swapchain(const vkx::VulkanDevice& device,
			  const vkx::VulkanRenderPass& renderPass,
			  const vkx::VulkanAllocator& allocator,
			  const vkx::SwapchainInfo& swapchainInfo,
			  vk::UniqueSwapchainKHR&& uniqueSwapchain)
    : logicalDevice(static_cast<VkDevice>(device)),
      swapchain(std::move(uniqueSwapchain)),
      imageExtent(swapchainInfo.actualExtent) {
	const auto images = logicalDevice.getSwapchainImagesKHR(*swapchain);

	const auto depthFormat = device.findDepthFormat();

	imageViews.reserve(images.size());
	for (const vk::Image image : images) {
		imageViews.emplace_back(device.createImageView(image, swapchainInfo.surfaceFormat, vk::ImageAspectFlagBits::eColor));
	}

	depthImage = allocator.allocateImage(imageExtent, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageView = depthImage.createView(depthFormat, vk::ImageAspectFlagBits::eDepth);
	
	framebuffers.reserve(imageViews.size());
	for (const auto& imageView : imageViews) {
		const std::array framebufferAttachments{*imageView, *depthImageView};

		const vk::FramebufferCreateInfo framebufferCreateInfo{
		    {},
		    static_cast<vk::RenderPass>(renderPass),
		    framebufferAttachments,
		    imageExtent.width,
		    imageExtent.height,
		    1};

		framebuffers.emplace_back(logicalDevice.createFramebufferUnique(framebufferCreateInfo));
	}
}

vk::ResultValue<std::uint32_t> vkx::Swapchain::acquireNextImage(const vkx::SyncObjects& syncObjects) const {
	return logicalDevice.acquireNextImageKHR(*swapchain, UINT64_MAX, *syncObjects.imageAvailableSemaphore);
}