#include <vkx/renderer/renderer.hpp>
#include <vkx/renderer/queue_config.hpp>
#include <vkx/renderer/swapchain_info.hpp>

namespace vkx {
void VulkanInstance::Swapchain::recreate() {
	const vkx::SwapchainInfo info{physicalDevice, surface, window};
	const vkx::QueueConfig config{physicalDevice, surface};

	imageExtent = info.actualExtent;

	int width;
	int height;
	SDL_GetWindowSizeInPixels(window, &width, &height);

	const auto imageSharingMode = config.getImageSharingMode();

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo{
	    {},
	    surface,
	    info.imageCount,
	    info.surfaceFormat,
	    info.surfaceColorSpace,
	    info.actualExtent,
	    1,
	    vk::ImageUsageFlagBits::eColorAttachment,
	    imageSharingMode,
	    config.indices,
	    info.currentTransform,
	    vk::CompositeAlphaFlagBitsKHR::eOpaque,
	    info.presentMode,
	    true};

	swapchain = logicalDevice.createSwapchainKHR(swapchainCreateInfo);

	const auto images = logicalDevice.getSwapchainImagesKHR(swapchain);

	imageViews.reserve(images.size());
	for (const vk::Image image : images) {
		const vk::ImageSubresourceRange subresourceRange{vk::ImageAspectFlagBits::eColor, 
			0,
			1,
			0,
			1};

		const vk::ImageViewCreateInfo imageViewCreateInfo{{},
			image,
			vk::ImageViewType::e2D,
			info.surfaceFormat,
			{},
			subresourceRange};

		imageViews.emplace_back(logicalDevice.createImageView(imageViewCreateInfo));
	}

	depthImage = vkx::Image{logicalDevice, allocator, imageExtent, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, VMA_MEMORY_USAGE_AUTO};
	depthImageView = depthImage.createView(depthFormat, vk::ImageAspectFlagBits::eDepth);
	
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

void VulkanInstance::Swapchain::destroy() {
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

vk::ResultValue<std::uint32_t> VulkanInstance::Swapchain::acquireNextImage(const vkx::SyncObjects& syncObjects) const {
	return logicalDevice.acquireNextImageKHR(swapchain, UINT64_MAX, *syncObjects.imageAvailableSemaphore);
}
}
