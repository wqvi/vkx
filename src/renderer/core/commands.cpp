#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::CommandSubmitter::CommandSubmitter(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface)
    : device(device) {
	const vkx::QueueConfig queueConfig{physicalDevice, surface};

	graphicsQueue = vkx::getObject<VkQueue>(vkGetDeviceQueue, device, *queueConfig.graphicsIndex, 0);
	presentQueue = vkx::getObject<VkQueue>(vkGetDeviceQueue, device, *queueConfig.presentIndex, 0);

	const VkCommandPoolCreateInfo commandPoolCreateInfo{
	    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    nullptr,
	    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	    *queueConfig.graphicsIndex};

	commandPool = vkx::create<VkCommandPool>(
	    vkCreateCommandPool,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create command pool");
		    }
	    },
	    device, &commandPoolCreateInfo, nullptr);
}

void vkx::CommandSubmitter::submitImmediately(const std::function<void(VkCommandBuffer)>& command) const {
	const VkCommandBufferAllocateInfo commandBufferAllocateInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    nullptr,
	    commandPool,
	    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    1};

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffer");
	}

	const VkCommandBufferBeginInfo commandBufferBeginInfo{
	    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    nullptr,
	    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	    nullptr};

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin command buffer");
	}

	command(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to end command buffer");
	}

	const VkSubmitInfo submitInfo{
	    VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    nullptr,
	    0,
	    nullptr,
	    nullptr,
	    1,
	    &commandBuffer,
	    0,
	    nullptr};

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr);
	vkQueueWaitIdle(graphicsQueue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void vkx::CommandSubmitter::copyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height) const {
	const VkImageSubresourceLayers subresourceLayer{
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    0,
	    0,
	    1};

	const VkOffset3D imageOffset{
	    0,
	    0,
	    0};

	const VkExtent3D imageExtent{
	    width,
	    height,
	    1};

	const VkBufferImageCopy region{
	    0,
	    0,
	    0,
	    subresourceLayer,
	    imageOffset,
	    imageExtent};

	submitImmediately([&buffer, &image, &region](auto commandBuffer) {
		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	});
}

void vkx::CommandSubmitter::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const {
	const VkImageSubresourceRange subresourceRange{
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    0,
	    1,
	    0,
	    1};

	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		srcAccessMask = 0;
		dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
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
	    oldLayout,
	    newLayout,
	    VK_QUEUE_FAMILY_IGNORED,
	    VK_QUEUE_FAMILY_IGNORED,
	    image,
	    subresourceRange};

	submitImmediately([&sourceStage, &destinationStage, &barrier](auto commandBuffer) {
		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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
	    &swapchain.swapchain,
	    &imageIndex,
	    nullptr};

	return vkQueuePresentKHR(presentQueue, &presentInfo);
}

void vkx::CommandSubmitter::destroy() const {
	vkDestroyCommandPool(device, commandPool, nullptr);
}