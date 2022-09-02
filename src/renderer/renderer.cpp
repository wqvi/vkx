#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Renderer::Renderer(const SDLWindow& window)
    : bootstrap(static_cast<SDL_Window*>(window)),
      device(bootstrap.createDevice()),
      allocator(device.createAllocator()),
      commandSubmitter(device.createCommandSubmitter()) {
	const vkx::SwapchainInfo swapchainInfo{device};
	const auto surfaceFormat = swapchainInfo.chooseSurfaceFormat().format;

	clearRenderPass = device.createRenderPass(surfaceFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear);
	loadRenderPass = device.createRenderPass(surfaceFormat, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eLoad);

	swapchain = device.createSwapchain(static_cast<SDL_Window*>(window), *clearRenderPass, allocator);

	syncObjects = device.createSyncObjects();
}

vkx::GraphicsPipeline* vkx::Renderer::attachPipeline(const vkx::GraphicsPipelineInformationTest& pipelineInformation) {
	pipelines.push_back(device.createGraphicsPipeline(*clearRenderPass, pipelineInformation));
	return &pipelines.front();
}

void vkx::Renderer::resized(const SDLWindow& window) {
	auto [newWidth, newHeight] = window.getSize();
	while (newWidth == 0 || newHeight == 0) {
		std::tie(newWidth, newHeight) = window.getSize();
		SDL_WaitEvent(nullptr);
	}

	device->waitIdle();

	swapchain = device.createSwapchain(static_cast<SDL_Window*>(window), *clearRenderPass, allocator);
}

vkx::Texture vkx::Renderer::createTexture(const std::string& file) const {
	return vkx::Texture{file, device, allocator, commandSubmitter};
}