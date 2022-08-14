#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/renderer_types.hpp>

vkx::CommandSubmitter::CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface)
    : device(device) {
	const vkx::QueueConfig queueConfig(physicalDevice, surface);

	graphicsQueue = device.getQueue(*queueConfig.graphicsIndex, 0);
	presentQueue = device.getQueue(*queueConfig.presentIndex, 0);

	const vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, *queueConfig.graphicsIndex);

	commandPool = device.createCommandPoolUnique(commandPoolInfo);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully created renderer command submitter.");
}

void vkx::CommandSubmitter::submitImmediately(const std::function<void(vk::CommandBuffer)>& command) const {
	const vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1);

	auto commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

	constexpr vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {}, nullptr);

	commandBuffer.begin(beginInfo);

	command(commandBuffer);

	commandBuffer.end();

	const vk::SubmitInfo submitInfo({}, {}, commandBuffer, {});

	static_cast<void>(graphicsQueue.submit(1, &submitInfo, {}));
	graphicsQueue.waitIdle();

	device.freeCommandBuffers(*commandPool, commandBuffer);
}

void vkx::CommandSubmitter::copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const {
	const vk::ImageSubresourceLayers subresourceLayer(vk::ImageAspectFlagBits::eColor, 0, 0, 1);

	const vk::Offset3D imageOffset(0, 0, 0);

	const vk::Extent3D imageExtent(width, height, 1);

	const vk::BufferImageCopy region(
	    0,
	    0,
	    0,
	    subresourceLayer,
	    imageOffset,
	    imageExtent);

	submitImmediately([&buffer, &image, &region](vk::CommandBuffer commandBuffer) {
		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	});
}

void vkx::CommandSubmitter::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
	const vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	vk::AccessFlags srcAccessMask;
	vk::AccessFlags dstAccessMask;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = vk::AccessFlagBits::eNone;
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else {
		throw std::invalid_argument("Unsupported layout transition.");
	}

	const vk::ImageMemoryBarrier barrier(
	    srcAccessMask,
	    dstAccessMask,
	    oldLayout,
	    newLayout,
	    VK_QUEUE_FAMILY_IGNORED,
	    VK_QUEUE_FAMILY_IGNORED,
	    image,
	    subresourceRange);

	submitImmediately([&sourceStage, &destinationStage, &barrier](vk::CommandBuffer commandBuffer) {
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
	});
}

std::vector<vk::CommandBuffer> vkx::CommandSubmitter::allocateDrawCommands(std::uint32_t amount) const {
	const vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, amount * MAX_FRAMES_IN_FLIGHT);

	return device.allocateCommandBuffers(allocInfo);
}

void vkx::CommandSubmitter::recordDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const {
	for (std::uint32_t i = 0; i < size; i++) {
		const auto commandBuffer = begin[i];
		const auto renderPass = drawInfo.renderPass[i];
		const auto vertexBuffer = drawInfo.vertexBuffers[i];
		const auto indexBuffer = drawInfo.indexBuffers[i];
		const auto indexCount = drawInfo.indexCount[i];

		commandBuffer.reset({});
		const vk::CommandBufferBeginInfo beginInfo{};
		static_cast<void>(commandBuffer.begin(beginInfo));

		constexpr std::array clearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
		constexpr vk::ClearColorValue clearColor(clearColorValue);
		constexpr vk::ClearDepthStencilValue clearDepthStencil(1.0f, 0);

		constexpr std::array<vk::ClearValue, 2> clearValues{clearColor, clearDepthStencil};

		const vk::Rect2D renderArea({0, 0}, drawInfo.extent);

		const vk::RenderPassBeginInfo renderPassInfo(renderPass, drawInfo.framebuffer, renderArea, clearValues);

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, drawInfo.graphicsPipeline);

		commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});

		commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, drawInfo.graphicsPipelineLayout, 0, drawInfo.descriptorSet, {});

		commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);

		commandBuffer.endRenderPass();

		commandBuffer.end();
	}
}

void vkx::CommandSubmitter::submitDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const {
	constexpr std::array waitStage{vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)};
	const vk::SubmitInfo submitInfo(
	    1,
	    &*syncObjects.imageAvailableSemaphore,
	    waitStage.data(),
	    size,
	    begin,
	    1,
	    &*syncObjects.renderFinishedSemaphore);

	graphicsQueue.submit(submitInfo, *syncObjects.inFlightFence);
}

vk::Result vkx::CommandSubmitter::presentToSwapchain(vk::SwapchainKHR swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const {
	const vk::PresentInfoKHR presentInfo(
	    1,
	    &*syncObjects.renderFinishedSemaphore,
	    1,
	    &swapchain,
	    &imageIndex);

	return presentQueue.presentKHR(&presentInfo);
}