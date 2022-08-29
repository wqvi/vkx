#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/renderer_types.hpp>

vkx::CommandSubmitter::CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface)
    : device(device) {
	const vkx::QueueConfig queueConfig{physicalDevice, surface};

	graphicsQueue = device.getQueue(*queueConfig.graphicsIndex, 0);
	presentQueue = device.getQueue(*queueConfig.presentIndex, 0);

	const vk::CommandPoolCreateInfo commandPoolInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, *queueConfig.graphicsIndex};

	commandPool = device.createCommandPoolUnique(commandPoolInfo);
}

void vkx::CommandSubmitter::submitImmediately(const std::function<void(vk::CommandBuffer)>& command) const {
	const vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1);

	auto commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

	constexpr vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {}, nullptr};

	commandBuffer.begin(beginInfo);

	command(commandBuffer);

	commandBuffer.end();

	const vk::SubmitInfo submitInfo({}, {}, commandBuffer, {});

	static_cast<void>(graphicsQueue.submit(1, &submitInfo, {}));
	graphicsQueue.waitIdle();

	device.freeCommandBuffers(*commandPool, commandBuffer);
}

void vkx::CommandSubmitter::copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const {
	const vk::ImageSubresourceLayers subresourceLayer{vk::ImageAspectFlagBits::eColor, 0, 0, 1};

	const vk::Offset3D imageOffset{0, 0, 0};

	const vk::Extent3D imageExtent{width, height, 1};

	const vk::BufferImageCopy region{
	    0,
	    0,
	    0,
	    subresourceLayer,
	    imageOffset,
	    imageExtent};

	submitImmediately([&buffer, &image, &region](vk::CommandBuffer commandBuffer) {
		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	});
}

void vkx::CommandSubmitter::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
	const vk::ImageSubresourceRange subresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

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

	const vk::ImageMemoryBarrier barrier{
	    srcAccessMask,
	    dstAccessMask,
	    oldLayout,
	    newLayout,
	    VK_QUEUE_FAMILY_IGNORED,
	    VK_QUEUE_FAMILY_IGNORED,
	    image,
	    subresourceRange};

	submitImmediately([&sourceStage, &destinationStage, &barrier](vk::CommandBuffer commandBuffer) {
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
	});
}

std::vector<vk::CommandBuffer> vkx::CommandSubmitter::allocateDrawCommands(std::uint32_t amount) const {
	const vk::CommandBufferAllocateInfo allocInfo{*commandPool, vk::CommandBufferLevel::ePrimary, amount * MAX_FRAMES_IN_FLIGHT};

	return device.allocateCommandBuffers(allocInfo);
}

std::vector<vk::CommandBuffer> vkx::CommandSubmitter::allocateSecondaryDrawCommands(std::uint32_t amount) const {
	const vk::CommandBufferAllocateInfo allocInfo{*commandPool, vk::CommandBufferLevel::eSecondary, amount * MAX_FRAMES_IN_FLIGHT};

	return device.allocateCommandBuffers(allocInfo);
}

void vkx::CommandSubmitter::recordPrimaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const {
	const auto extent = drawInfo.swapchain.lock()->extent();
	const auto framebuffer = (*drawInfo.swapchain.lock())[drawInfo.imageIndex];
	
	constexpr vk::CommandBufferBeginInfo beginInfo{};
	
	constexpr std::array clearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
	constexpr vk::ClearColorValue clearColor{clearColorValue};
	constexpr vk::ClearDepthStencilValue clearDepthStencil{1.0f, 0};
	constexpr std::array<vk::ClearValue, 2> clearValues = {clearColor, clearDepthStencil};
	const vk::Rect2D renderArea{{0, 0}, extent};
	const vk::RenderPassBeginInfo renderPassInfo{drawInfo.renderPass, framebuffer, renderArea, clearValues};

	const vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f);

	const vk::Rect2D scissor({0, 0}, extent);

	for (std::uint32_t i = 0; i < size; i++) {
		const auto commandBuffer = begin[i];
		const auto vertexBuffer = drawInfo.vertexBuffers[i];
		const auto indexBuffer = drawInfo.indexBuffers[i];
		const auto indexCount = drawInfo.indexCount[i];

		commandBuffer.reset({});
		static_cast<void>(commandBuffer.begin(beginInfo));

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline.lock()->pipeline);

		commandBuffer.setViewport(0, viewport);

		commandBuffer.setScissor(0, scissor);

		commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});

		commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline.lock()->layout, 0, drawInfo.descriptorSet, {});

		commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);

		commandBuffer.endRenderPass();

		commandBuffer.end();
	}
}

void vkx::CommandSubmitter::recordSecondaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vk::CommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const {
	const auto extent = drawInfo.swapchain.lock()->extent();
	const auto framebuffer = (*drawInfo.swapchain.lock())[drawInfo.imageIndex];

	constexpr vk::CommandBufferBeginInfo beginInfo{};
	const vk::CommandBufferInheritanceInfo secondaryInheritanceInfo{drawInfo.renderPass, 0, framebuffer};
	const vk::CommandBufferBeginInfo secondaryBeginInfo{vk::CommandBufferUsageFlagBits::eRenderPassContinue, &secondaryInheritanceInfo};

	constexpr std::array clearColorValue = {0.0f, 0.0f, 0.0f, 1.0f};
	constexpr vk::ClearColorValue clearColor{clearColorValue};
	constexpr vk::ClearDepthStencilValue clearDepthStencil{1.0f, 0};
	constexpr std::array<vk::ClearValue, 2> clearValues = {clearColor, clearDepthStencil};
	const vk::Rect2D renderArea{{0, 0}, extent};
	const vk::RenderPassBeginInfo renderPassInfo{drawInfo.renderPass, framebuffer, renderArea, clearValues};

	const vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f);

	const vk::Rect2D scissor({0, 0}, extent);

	for (std::uint32_t i = 0; i < size; i++) {
		const auto commandBuffer = begin[i];

		commandBuffer.reset({});
		static_cast<void>(commandBuffer.begin(beginInfo));

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);

		for (std::uint32_t j = 0; j < secondarySize; j++) {
			const auto secondaryCommandBuffer = secondaryBegin[j];
			const auto vertexBuffer = drawInfo.vertexBuffers[j];
			const auto indexBuffer = drawInfo.indexBuffers[j];
			const auto indexCount = drawInfo.indexCount[j];

			static_cast<void>(secondaryCommandBuffer.begin(secondaryBeginInfo));

			secondaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline.lock()->pipeline);

			secondaryCommandBuffer.setViewport(0, viewport);

			secondaryCommandBuffer.setScissor(0, scissor);

			secondaryCommandBuffer.bindVertexBuffers(0, vertexBuffer, {0});

			secondaryCommandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

			secondaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline.lock()->layout, 0, drawInfo.descriptorSet, {});

			secondaryCommandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);

			secondaryCommandBuffer.end();
		}

		commandBuffer.executeCommands(secondarySize, secondaryBegin);

		commandBuffer.endRenderPass();

		commandBuffer.end();
	}
}

void vkx::CommandSubmitter::submitDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const {
	constexpr std::array waitStage = {vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)};
	const vk::SubmitInfo submitInfo{
	    1,
	    &*syncObjects.imageAvailableSemaphore,
	    waitStage.data(),
	    size,
	    begin,
	    1,
	    &*syncObjects.renderFinishedSemaphore};

	graphicsQueue.submit(submitInfo, *syncObjects.inFlightFence);
}

vk::Result vkx::CommandSubmitter::presentToSwapchain(const Swapchain& swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const {
	const vk::PresentInfoKHR presentInfo{
	    1,
	    &*syncObjects.renderFinishedSemaphore,
	    1,
	    &*swapchain.swapchain,
	    &imageIndex};

	return presentQueue.presentKHR(&presentInfo);
}