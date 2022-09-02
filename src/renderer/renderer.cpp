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

void vkx::Renderer::createDrawCommands(const std::vector<DrawInfoTest>& drawInfos) {
	if (drawInfos.empty()) {
		throw std::invalid_argument("Draw infos can't be empty.");
	}

	for (const auto& drawInfo : drawInfos) {
		if (drawInfo.level == vk::CommandBufferLevel::eSecondary) {
			secondaryDrawCommandsAmount += drawInfo.meshes.size();
		}
	}

	primaryCommandsBuffers = commandSubmitter.allocateDrawCommands(drawInfos.size());

	if (secondaryDrawCommandsAmount != 0) {
		secondaryCommandBuffers = commandSubmitter.allocateSecondaryDrawCommands(secondaryDrawCommandsAmount);
	}

	primaryDrawCommandsAmount = drawInfos.size();
}

void vkx::Renderer::lazySync(const vkx::SDLWindow& window) {
	const auto& syncObject = syncObjects[currentFrame];
	syncObject.waitForFence();
	vk::Result result = vk::Result::eSuccess;
	std::tie(result, imageIndex) = swapchain.acquireNextImage(device, syncObject);

	if (result == vk::Result::eErrorOutOfDateKHR) {
		resized(window);
	} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("Failed to acquire next image.");
	}

	syncObject.resetFence();
}

void vkx::Renderer::uploadDrawCommands(const std::vector<DrawInfoTest>& drawInfos, const SDLWindow& window) {
	const auto& syncObject = syncObjects[currentFrame];

	const vk::CommandBuffer* primaryBegin = &primaryCommandsBuffers[currentFrame * primaryDrawCommandsAmount];

	const vk::CommandBuffer* secondaryBegin = &secondaryCommandBuffers[currentFrame * secondaryDrawCommandsAmount];

	std::uintptr_t offset = 0;

	for (std::uint32_t i = 0; i < drawInfos.size(); i++) {
		const auto& drawInfo = drawInfos[i];

		const vk::CommandBuffer* primary = primaryBegin + static_cast<std::uintptr_t>(i);

		const vk::CommandBuffer* secondary = secondaryBegin + offset;

		if (drawInfo.level == vk::CommandBufferLevel::ePrimary) {
			commandSubmitter.recordPrimaryDrawCommands(nullptr, 0, {});
		} else {
			commandSubmitter.recordSecondaryDrawCommands(nullptr, 0, nullptr, 0, {});
		}

		offset += static_cast<std::uintptr_t>(drawInfo.meshes.size());
	}

	commandSubmitter.submitDrawCommands(primaryBegin, primaryDrawCommandsAmount, syncObject);
	
	const auto result = commandSubmitter.presentToSwapchain(swapchain, imageIndex, syncObject);
	
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;

		resized(window);
	} else if (result != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to present.");
	}
}

void vkx::Renderer::lazyUpdate() {
	currentFrame = (currentFrame + 1) % vkx::MAX_FRAMES_IN_FLIGHT;
}