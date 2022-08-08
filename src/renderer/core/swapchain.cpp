#include <vkx/renderer/core/swapchain.hpp>

#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>

vkx::Swapchain::Swapchain() = default;

vkx::Swapchain::Swapchain(const Device& device, const vk::UniqueSurfaceKHR& surface, SDL_Window* window, const Swapchain& oldSwapchain) {
	const SwapchainInfo info{static_cast<vk::PhysicalDevice>(device), surface};
	const QueueConfig config{static_cast<vk::PhysicalDevice>(device), surface};

	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);

	const auto surfaceFormat = info.chooseSurfaceFormat();
	const auto presentMode = info.choosePresentMode();
	const auto actualExtent = info.chooseExtent(width, height);
	const auto imageCount = info.getImageCount();
	const auto imageSharingMode = config.getImageSharingMode();

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo(
	    {},
	    *surface,
	    imageCount,
	    surfaceFormat.format,
	    surfaceFormat.colorSpace,
	    actualExtent,
	    1,
	    vk::ImageUsageFlagBits::eColorAttachment,
	    imageSharingMode,
	    config.indices,
	    info.capabilities.currentTransform,
	    vk::CompositeAlphaFlagBitsKHR::eOpaque,
	    presentMode,
	    true);

	swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);

	images = device->getSwapchainImagesKHR(*swapchain);

	imageFormat = surfaceFormat.format;
	extent = actualExtent;

	for (const auto image : images) {
		imageViews.push_back(device.createImageViewUnique(image, imageFormat, vk::ImageAspectFlagBits::eColor));
	}

	const auto depthFormat = device.findDepthFormat();
	depthImage = device.createImageUnique(extent.width, extent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageMemory = device.allocateMemoryUnique(depthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
	depthImageView = device.createImageViewUnique(*depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

vkx::Swapchain::operator const vk::SwapchainKHR&() const { return *swapchain; }

vkx::Swapchain::operator const vk::UniqueSwapchainKHR&() const { return swapchain; }

void vkx::Swapchain::createFramebuffers(const Device& device, const vk::UniqueRenderPass& renderPass) {
	framebuffers.resize(imageViews.size());

	for (std::size_t i = 0; i < imageViews.size(); i++) {
		const std::array framebufferAttachments{*imageViews[i], *depthImageView};

		const vk::FramebufferCreateInfo framebufferInfo(
		    {},
		    *renderPass,
		    framebufferAttachments,
		    extent.width,
		    extent.height,
		    1);

		framebuffers[i] = device->createFramebufferUnique(framebufferInfo);
	}
}

vk::ResultValue<std::uint32_t> vkx::Swapchain::acquireNextImage(const Device& device, const vk::UniqueSemaphore& semaphore) const {
	std::uint32_t imageIndex = 0;
	const auto result = device->acquireNextImageKHR(*swapchain, UINT64_MAX, *semaphore, {}, &imageIndex);
	
  return {result, imageIndex};
}