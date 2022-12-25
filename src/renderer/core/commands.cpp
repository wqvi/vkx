#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::CommandSubmitter::CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface)
    : device(device) {
	const vkx::QueueConfig queueConfig{physicalDevice, surface};

	graphicsQueue = device.getQueue(*queueConfig.graphicsIndex, 0);
	presentQueue = device.getQueue(*queueConfig.presentIndex, 0);

	const vk::CommandPoolCreateInfo commandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, *queueConfig.graphicsIndex};

	commandPool = device.createCommandPoolUnique(commandPoolCreateInfo);
}

void vkx::CommandSubmitter::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
	const vk::ImageSubresourceRange subresourceRange{
	    vk::ImageAspectFlagBits::eColor,
	    0,
	    1,
	    0,
	    1};

	vk::AccessFlags srcAccessMask;
	vk::AccessFlags dstAccessMask;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = {};
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

	submitImmediately([&sourceStage, &destinationStage, &barrier](auto commandBuffer) {
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
	});
}

void vkx::CommandSubmitter::copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const {
	const vk::ImageSubresourceLayers subresourceLayer{
	    vk::ImageAspectFlagBits::eColor,
	    0,
	    0,
	    1};

	const vk::Offset3D imageOffset{
	    0,
	    0,
	    0};

	const vk::Extent3D imageExtent{
	    width,
	    height,
	    1};

	const vk::BufferImageCopy region{
	    0,
	    0,
	    0,
	    subresourceLayer,
	    imageOffset,
	    imageExtent};

	submitImmediately([&buffer, &image, &region](auto commandBuffer) {
		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	});
}

std::vector<vk::CommandBuffer> vkx::CommandSubmitter::allocateDrawCommands(std::uint32_t amount) const {
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
	    *commandPool,
	    vk::CommandBufferLevel::ePrimary,
	    amount * vkx::MAX_FRAMES_IN_FLIGHT};

	return device.allocateCommandBuffers(commandBufferAllocateInfo);
}

std::vector<vk::CommandBuffer> vkx::CommandSubmitter::allocateSecondaryDrawCommands(std::uint32_t amount) const {
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
	    *commandPool,
	    vk::CommandBufferLevel::eSecondary,
	    amount * vkx::MAX_FRAMES_IN_FLIGHT};

	return device.allocateCommandBuffers(commandBufferAllocateInfo);
}

void vkx::CommandSubmitter::recordPrimaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const {
	const auto extent = drawInfo.swapchain->extent();
	const auto framebuffer = (*drawInfo.swapchain)[drawInfo.imageIndex];

	const vk::CommandBufferBeginInfo commandBufferBeginInfo{};

	const vk::Rect2D renderArea{
	    {0, 0},
	    extent};

	constexpr std::array clearColor{0.0f, 0.0f, 0.0f, 1.0f};
	const vk::ClearDepthStencilValue clearDepthStencil{1.0f, 0};

	const std::vector<vk::ClearValue> clearValues{vk::ClearValue{clearColor}, vk::ClearValue{clearDepthStencil}};

	const vk::RenderPassBeginInfo renderPassBeginInfo{
	    drawInfo.renderPass,
	    framebuffer,
	    renderArea,
	    clearValues};

	const vk::Viewport viewport{
	    0.0f,
	    0.0f,
	    static_cast<float>(extent.width),
	    static_cast<float>(extent.height),
	    0.0f,
	    1.0f};

	for (std::uint32_t i = 0; i < size; i++) {
		const auto commandBuffer = begin[i];
		const vk::Buffer vertexBuffer = drawInfo.vertexBuffers[i];
		const vk::Buffer indexBuffer = drawInfo.indexBuffers[i];
		const auto indexCount = drawInfo.indexCount[i];

		commandBuffer.reset();
		commandBuffer.begin(commandBufferBeginInfo);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, drawInfo.graphicsPipeline->pipeline);

		commandBuffer.setViewport(0, viewport);

		commandBuffer.setScissor(0, renderArea);

		commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});

		commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, static_cast<vk::PipelineLayout>(drawInfo.graphicsPipeline->pipelineLayout), 0, static_cast<vk::DescriptorSet>(drawInfo.graphicsPipeline->descriptorSets[drawInfo.currentFrame]), {});

		commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);

		commandBuffer.endRenderPass();

		commandBuffer.end();
	}
}

void vkx::CommandSubmitter::recordSecondaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vk::CommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const {
	const auto extent = drawInfo.swapchain->extent();
	const auto framebuffer = (*drawInfo.swapchain)[drawInfo.imageIndex];

	const vk::CommandBufferBeginInfo commandBufferBeginInfo{};

	const vk::CommandBufferInheritanceInfo secondaryCommandBufferInheritanceInfo{
	    drawInfo.renderPass,
	    0,
	    framebuffer};

	const vk::CommandBufferBeginInfo secondaryCommandBufferBeginInfo{
	    vk::CommandBufferUsageFlagBits::eRenderPassContinue,
	    &secondaryCommandBufferInheritanceInfo};

	const vk::Rect2D renderArea{
	    {0, 0},
	    extent};

	constexpr std::array clearColor{0.0f, 0.0f, 0.0f, 1.0f};
	const vk::ClearDepthStencilValue clearDepthStencil{1.0f, 0};

	const std::vector<vk::ClearValue> clearValues{vk::ClearValue{clearColor}, vk::ClearValue{clearDepthStencil}};

	const vk::RenderPassBeginInfo renderPassBeginInfo{
	    drawInfo.renderPass,
	    framebuffer,
	    renderArea,
	    clearValues};

	const vk::Viewport viewport{
	    0.0f,
	    0.0f,
	    static_cast<float>(extent.width),
	    static_cast<float>(extent.height),
	    0.0f,
	    1.0f};

	for (std::uint32_t i = 0; i < size; i++) {
		const auto commandBuffer = begin[i];

		commandBuffer.reset();
		commandBuffer.begin(commandBufferBeginInfo);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eSecondaryCommandBuffers);

		for (std::uint32_t j = 0; j < secondarySize; j++) {
			const auto secondaryCommandBuffer = secondaryBegin[j];
			const vk::Buffer vertexBuffer = drawInfo.vertexBuffers[j];
			const vk::Buffer indexBuffer = drawInfo.indexBuffers[j];
			const auto indexCount = drawInfo.indexCount[j];

			secondaryCommandBuffer.begin(secondaryCommandBufferBeginInfo);

			secondaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, drawInfo.graphicsPipeline->pipeline);

			secondaryCommandBuffer.setViewport(0, viewport);

			secondaryCommandBuffer.setScissor(0, renderArea);

			secondaryCommandBuffer.bindVertexBuffers(0, vertexBuffer, {0});

			secondaryCommandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

			secondaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, static_cast<vk::PipelineLayout>(drawInfo.graphicsPipeline->pipelineLayout), 0, static_cast<vk::DescriptorSet>(drawInfo.graphicsPipeline->descriptorSets[drawInfo.currentFrame]), {});

			secondaryCommandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);

			secondaryCommandBuffer.end();
		}

		commandBuffer.executeCommands(secondarySize, secondaryBegin);

		commandBuffer.endRenderPass();

		commandBuffer.end();
	}
}

void vkx::CommandSubmitter::submitDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const {
	constexpr std::array<vk::PipelineStageFlags, 1> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};

	const vk::SubmitInfo submitInfo{
	    1,
	    reinterpret_cast<const vk::Semaphore*>(&syncObjects.imageAvailableSemaphore),
	    waitStages.data(),
	    size,
	    begin,
	    1,
	    reinterpret_cast<const vk::Semaphore*>(&syncObjects.renderFinishedSemaphore)};

	graphicsQueue.submit(submitInfo, syncObjects.inFlightFence);
}

vk::Result vkx::CommandSubmitter::presentToSwapchain(const Swapchain& swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const {
	const vk::PresentInfoKHR presentInfo{
	    1,
	    reinterpret_cast<const vk::Semaphore*>(&syncObjects.renderFinishedSemaphore),
	    1,
		&*swapchain.swapchain,
	    &imageIndex,
	    nullptr};

	return presentQueue.presentKHR(presentInfo);
}