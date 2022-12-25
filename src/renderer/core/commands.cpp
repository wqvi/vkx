#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::CommandSubmitter::CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface)
    : device(device) {
	const vkx::QueueConfig queueConfig{physicalDevice, surface};

	graphicsQueue = device.getQueue(*queueConfig.graphicsIndex, 0);
	presentQueue = device.getQueue(*queueConfig.presentIndex, 0);

	const vk::CommandPoolCreateInfo commandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, *queueConfig.graphicsIndex};

	commandPool = device.createCommandPool(commandPoolCreateInfo);
}

void vkx::CommandSubmitter::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
	const vk::ImageSubresourceRange subresourceRange{
	    vk::ImageAspectFlagBits::eColor,
	    0,
	    1,
	    0,
	    1};

	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = 0;
		dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		throw std::invalid_argument("Unsupported layout transition.");
	}

	const VkImageMemoryBarrier barrier{
	    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	    nullptr,
	    srcAccessMask,
	    dstAccessMask,
	    static_cast<VkImageLayout>(oldLayout),
	    static_cast<VkImageLayout>(newLayout),
	    VK_QUEUE_FAMILY_IGNORED,
	    VK_QUEUE_FAMILY_IGNORED,
	    image,
	    static_cast<VkImageSubresourceRange>(subresourceRange)};

	submitImmediately([&sourceStage, &destinationStage, &barrier](auto commandBuffer) {
		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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

std::vector<VkCommandBuffer> vkx::CommandSubmitter::allocateDrawCommands(std::uint32_t amount) const {
	const VkCommandBufferAllocateInfo commandBufferAllocateInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    nullptr,
	    commandPool,
	    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    amount * vkx::MAX_FRAMES_IN_FLIGHT};

	std::vector<VkCommandBuffer> commandBuffers{amount * vkx::MAX_FRAMES_IN_FLIGHT};
	if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate draw commands");
	}

	return commandBuffers;
}

std::vector<VkCommandBuffer> vkx::CommandSubmitter::allocateSecondaryDrawCommands(std::uint32_t amount) const {
	const VkCommandBufferAllocateInfo commandBufferAllocateInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    nullptr,
	    commandPool,
	    VK_COMMAND_BUFFER_LEVEL_SECONDARY,
	    amount * vkx::MAX_FRAMES_IN_FLIGHT};

	std::vector<VkCommandBuffer> commandBuffers{amount * vkx::MAX_FRAMES_IN_FLIGHT};
	if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate draw commands");
	}

	return commandBuffers;
}

void vkx::CommandSubmitter::recordPrimaryDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const {
	const auto extent = drawInfo.swapchain->extent();
	const auto framebuffer = (*drawInfo.swapchain)[drawInfo.imageIndex];

	const VkCommandBufferBeginInfo commandBufferBeginInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    nullptr,
	    0,
	    nullptr};

	const VkRect2D renderArea{
	    {0, 0},
	    extent};

	const VkClearColorValue clearColor{0.0f, 0.0f, 0.0f, 1.0f};
	const VkClearDepthStencilValue clearDepthStencil{1.0f, 0};

	VkClearValue clearValues[2];
	clearValues[0].color = clearColor;
	clearValues[1].depthStencil = clearDepthStencil;

	const VkRenderPassBeginInfo renderPassBeginInfo{
	    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	    nullptr,
	    drawInfo.renderPass,
	    framebuffer,
	    renderArea,
	    2,
	    clearValues};

	const VkViewport viewport{
	    0.0f,
	    0.0f,
	    static_cast<float>(extent.width),
	    static_cast<float>(extent.height),
	    0.0f,
	    1.0f};

	for (std::uint32_t i = 0; i < size; i++) {
		const auto commandBuffer = begin[i];
		const auto vertexBuffer = drawInfo.vertexBuffers[i];
		const auto indexBuffer = drawInfo.indexBuffers[i];
		const auto indexCount = drawInfo.indexCount[i];

		vkResetCommandBuffer(commandBuffer, 0);
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.graphicsPipeline->pipeline);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		vkCmdSetScissor(commandBuffer, 0, 1, &renderArea);

		const VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.graphicsPipeline->pipelineLayout, 0, 1, &drawInfo.graphicsPipeline->descriptorSets[drawInfo.currentFrame], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		vkEndCommandBuffer(commandBuffer);
	}
}

void vkx::CommandSubmitter::recordSecondaryDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const VkCommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const {
	const auto extent = drawInfo.swapchain->extent();
	const auto framebuffer = (*drawInfo.swapchain)[drawInfo.imageIndex];

	const VkCommandBufferBeginInfo commandBufferBeginInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    nullptr,
	    0,
	    nullptr};

	const VkCommandBufferInheritanceInfo secondaryCommandBufferInheritanceInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
	    nullptr,
	    drawInfo.renderPass,
	    0,
	    framebuffer};

	const VkCommandBufferBeginInfo secondaryCommandBufferBeginInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    nullptr,
	    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
	    &secondaryCommandBufferInheritanceInfo};

	const VkRect2D renderArea{
	    {0, 0},
	    static_cast<VkExtent2D>(extent)};

	const VkClearColorValue clearColor{0.0f, 0.0f, 0.0f, 1.0f};
	const VkClearDepthStencilValue clearDepthStencil{1.0f, 0};

	VkClearValue clearValues[2];
	clearValues[0].color = clearColor;
	clearValues[1].depthStencil = clearDepthStencil;

	const VkRenderPassBeginInfo renderPassBeginInfo{
	    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	    nullptr,
	    drawInfo.renderPass,
	    framebuffer,
	    renderArea,
	    2,
	    clearValues};

	const VkViewport viewport{
	    0.0f,
	    0.0f,
	    static_cast<float>(extent.width),
	    static_cast<float>(extent.height),
	    0.0f,
	    1.0f};

	for (std::uint32_t i = 0; i < size; i++) {
		const auto commandBuffer = begin[i];

		vkResetCommandBuffer(commandBuffer, 0);
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		for (std::uint32_t j = 0; j < secondarySize; j++) {
			const auto secondaryCommandBuffer = secondaryBegin[j];
			const auto vertexBuffer = drawInfo.vertexBuffers[j];
			const auto indexBuffer = drawInfo.indexBuffers[j];
			const auto indexCount = drawInfo.indexCount[j];

			vkBeginCommandBuffer(secondaryCommandBuffer, &secondaryCommandBufferBeginInfo);

			vkCmdBindPipeline(secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.graphicsPipeline->pipeline);

			vkCmdSetViewport(secondaryCommandBuffer, 0, 1, &viewport);

			vkCmdSetScissor(secondaryCommandBuffer, 0, 1, &renderArea);

			const VkDeviceSize offsets[1] = {0};
			vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, &vertexBuffer, offsets);

			vkCmdBindIndexBuffer(secondaryCommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.graphicsPipeline->pipelineLayout, 0, 1, &drawInfo.graphicsPipeline->descriptorSets[drawInfo.currentFrame], 0, nullptr);

			vkCmdDrawIndexed(secondaryCommandBuffer, indexCount, 1, 0, 0, 0);

			vkEndCommandBuffer(secondaryCommandBuffer);
		}

		vkCmdExecuteCommands(commandBuffer, secondarySize, secondaryBegin);

		vkCmdEndRenderPass(commandBuffer);

		vkEndCommandBuffer(commandBuffer);
	}
}

void vkx::CommandSubmitter::submitDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const {
	const VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	const VkSubmitInfo submitInfo{
	    VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    nullptr,
	    1,
	    &syncObjects.imageAvailableSemaphore,
	    waitStages,
	    size,
	    begin,
	    1,
	    &syncObjects.renderFinishedSemaphore};

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, syncObjects.inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw commands");
	}
}

VkResult vkx::CommandSubmitter::presentToSwapchain(const Swapchain& swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const {
	const VkPresentInfoKHR presentInfo{
	    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	    nullptr,
	    1,
	    &syncObjects.renderFinishedSemaphore,
	    1,
	    reinterpret_cast<const VkSwapchainKHR*>(&*swapchain.swapchain),
	    &imageIndex,
	    nullptr};

	return vkQueuePresentKHR(presentQueue, &presentInfo);
}

void vkx::CommandSubmitter::destroy() const {
	vkDestroyCommandPool(device, commandPool, nullptr);
}