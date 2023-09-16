#include <vkx/renderer/swapchain.hpp>

vkx::Swapchain::Swapchain(const vkx::VulkanInstance& instance,
			  vk::RenderPass renderPass,
			  const vkx::SwapchainInfo& swapchainInfo,
			  vk::SwapchainKHR swapchain)
    : logicalDevice(instance.logicalDevice),
      swapchain(swapchain),
      imageExtent(swapchainInfo.actualExtent) {
	const auto images = logicalDevice.getSwapchainImagesKHR(swapchain);

	imageViews.reserve(images.size());
	for (const vk::Image image : images) {
		imageViews.emplace_back(instance.createImageView(image, swapchainInfo.surfaceFormat, vk::ImageAspectFlagBits::eColor));
	}

	depthImage = instance.allocateImage(imageExtent, instance.depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageView = depthImage.createView(instance.depthFormat, vk::ImageAspectFlagBits::eDepth);
	
	framebuffers.reserve(imageViews.size());
	for (const auto& imageView : imageViews) {
		const std::array framebufferAttachments{imageView, depthImageView};

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

void vkx::Swapchain::destroy() {
	for (auto& view : imageViews) {
		logicalDevice.destroyImageView(view);
	}
	
	for (auto& framebuffer : framebuffers) {
		logicalDevice.destroyFramebuffer(framebuffer);
	}

	logicalDevice.destroyImageView(depthImageView);
	depthImage.destroy();
	
	logicalDevice.destroySwapchainKHR(swapchain);
}

vk::ResultValue<std::uint32_t> vkx::Swapchain::acquireNextImage(const vkx::SyncObjects& syncObjects) const {
	return logicalDevice.acquireNextImageKHR(swapchain, UINT64_MAX, *syncObjects.imageAvailableSemaphore);
}
