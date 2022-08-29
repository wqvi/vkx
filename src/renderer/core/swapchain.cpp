#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>

vkx::Swapchain::Swapchain(const Device& device, vk::SurfaceKHR surface, SDL_Window* window, const std::shared_ptr<Allocator>& allocator) {
	if (!static_cast<bool>(surface)) {
		throw std::invalid_argument("Surface must be a valid handle to create a swapchain.");
	}

	const SwapchainInfo info{device};
	const QueueConfig config{device};

	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);

	const auto surfaceFormat = info.chooseSurfaceFormat();
	const auto presentMode = info.choosePresentMode();
	const auto actualExtent = info.chooseExtent(width, height);
	const auto imageCount = info.getImageCount();
	const auto imageSharingMode = config.getImageSharingMode();

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo{
	    {},
	    surface,
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
	    true};

	swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);

	images = device->getSwapchainImagesKHR(*swapchain);

	imageFormat = surfaceFormat.format;
	imageExtent = actualExtent;

	for (const auto image : images) {
		imageViews.push_back(device.createImageViewUnique(image, imageFormat, vk::ImageAspectFlagBits::eColor));
	}

	const auto depthFormat = device.findDepthFormat();
	depthResource = allocator->allocateImage(imageExtent.width, imageExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageView = device.createImageViewUnique(depthResource->object, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void vkx::Swapchain::createFramebuffers(const vkx::Device& device, vk::RenderPass renderPass) {
	framebuffers.resize(imageViews.size());

	for (std::size_t i = 0; i < imageViews.size(); i++) {
		const std::array framebufferAttachments = {*imageViews[i], *depthImageView};

		const vk::FramebufferCreateInfo framebufferInfo{
		    {},
		    renderPass,
		    framebufferAttachments,
		    imageExtent.width,
		    imageExtent.height,
		    1};

		framebuffers[i] = device->createFramebufferUnique(framebufferInfo);
	}
}

vk::ResultValue<std::uint32_t> vkx::Swapchain::acquireNextImage(const vkx::Device& device, const vkx::SyncObjects& syncObjects) const {
	std::uint32_t imageIndex = 0;
	const auto result = device->acquireNextImageKHR(*swapchain, UINT64_MAX, *syncObjects.imageAvailableSemaphore, {}, &imageIndex);

	return {result, imageIndex};
}