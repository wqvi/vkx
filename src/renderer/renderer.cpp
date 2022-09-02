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
}

void vkx::Renderer::attachPipeline(const vkx::GraphicsPipelineInformationTest& pipelineInformation) {
	pipelines.push_back(device.createGraphicsPipeline(*clearRenderPass, pipelineInformation));
}