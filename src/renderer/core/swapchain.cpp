#include "vkx/renderer/core/renderer_base.hpp"
#include <SDL2/SDL_log.h>
#include <vkx/renderer/core/swapchain.hpp>

#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vulkan/vulkan_handles.hpp>

vkx::Swapchain::Swapchain() = default;

vkx::Swapchain::Swapchain(const Device& device, vk::SurfaceKHR surface, SDL_Window* window, const std::shared_ptr<Allocator>& allocator) {
	if (!static_cast<bool>(surface)) {
		throw std::invalid_argument("Surface must be a valid handle to create a swapchain.");
	}

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
	    true);

	swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);

	images = device->getSwapchainImagesKHR(*swapchain);

	imageFormat = surfaceFormat.format;
	extent = actualExtent;

	for (const auto image : images) {
		imageViews.push_back(device.createImageViewUnique(image, imageFormat, vk::ImageAspectFlagBits::eColor));
	}

	const auto depthFormat = device.findDepthFormat();
	depthResource = allocator->allocateImage(extent.width, extent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthImageView = device.createImageViewUnique(depthResource->object, depthFormat, vk::ImageAspectFlagBits::eDepth);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully created renderer swapchain.");
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

void vkx::Swapchain::createFramebuffers(vk::Device device, vk::RenderPass renderPass) {
	framebuffers.resize(imageViews.size());

	for (std::size_t i = 0; i < imageViews.size(); i++) {
		const std::array framebufferAttachments{*imageViews[i], *depthImageView};

		const vk::FramebufferCreateInfo framebufferInfo(
		    {},
		    renderPass,
		    framebufferAttachments,
		    extent.width,
		    extent.height,
		    1);

		framebuffers[i] = device.createFramebufferUnique(framebufferInfo);
	}
}

vk::ResultValue<std::uint32_t> vkx::Swapchain::acquireNextImage(const Device& device, const vk::UniqueSemaphore& semaphore) const {
	std::uint32_t imageIndex = 0;
	const auto result = device->acquireNextImageKHR(*swapchain, UINT64_MAX, *semaphore, {}, &imageIndex);

	return {result, imageIndex};
}

vk::ResultValue<std::uint32_t> vkx::Swapchain::acquireNextImage(vk::Device device, const vkx::SyncObjects& syncObjects) const {
	std::uint32_t imageIndex = 0;
	const auto result = device.acquireNextImageKHR(*swapchain, UINT64_MAX, *syncObjects.imageAvailableSemaphore, {}, &imageIndex);

	return {result, imageIndex};
}