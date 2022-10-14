#include "vkx/renderer/renderer.hpp"
#include <vkx/renderer/core/swapchain.hpp>
#include <vulkan/vulkan_enums.hpp>

vkx::Swapchain::Swapchain(vk::Device device, vk::PhysicalDevice physicalDevice, vk::RenderPass renderPass, vk::SurfaceKHR surface, SDL_Window* window, const Allocator& allocator) {
	if (!static_cast<bool>(surface)) {
		throw std::invalid_argument("Surface must be a valid handle to create a swapchain.");
	}

	const SwapchainInfo info{physicalDevice, surface};
	const QueueConfig config{physicalDevice, surface};

	swapchain = vkx::createSwapchainUnique(device, surface, window, info, config);

	images = device.getSwapchainImagesKHR(*swapchain);

	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);

	const auto imageFormat = info.chooseSurfaceFormat().format;
	imageExtent = info.chooseExtent(width, height);

	for (const auto image : images) {
		imageViews.push_back(vkx::createImageViewUnique(static_cast<vk::Device>(device), image, imageFormat, vk::ImageAspectFlagBits::eColor));
	}

	const auto depthFormat = vkx::findDepthFormat(physicalDevice);
	depthResource = allocator.allocateImage(imageExtent.width, imageExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageView = vkx::createImageViewUnique(static_cast<vk::Device>(device), depthResource->object, depthFormat, vk::ImageAspectFlagBits::eDepth);

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

		framebuffers[i] = device.createFramebufferUnique(framebufferInfo);
	}
}

vk::ResultValue<std::uint32_t> vkx::Swapchain::acquireNextImage(const vkx::Device& device, const vkx::SyncObjects& syncObjects) const {
	std::uint32_t imageIndex = 0;
	const auto result = device->acquireNextImageKHR(*swapchain, UINT64_MAX, *syncObjects.imageAvailableSemaphore, {}, &imageIndex);

	return {result, imageIndex};
}