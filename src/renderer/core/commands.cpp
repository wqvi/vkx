#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/model.hpp>

vkx::CommandSubmitter::CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::SurfaceKHR surface)
    : logicalDevice(logicalDevice) {
	const vkx::QueueConfig queueConfig{physicalDevice, surface};

	const vk::CommandPoolCreateInfo commandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, *queueConfig.graphicsIndex};

	commandPool = logicalDevice.createCommandPoolUnique(commandPoolCreateInfo);

	graphicsQueue = logicalDevice.getQueue(*queueConfig.graphicsIndex, 0);
	presentQueue = logicalDevice.getQueue(*queueConfig.presentIndex, 0);
}

void vkx::CommandSubmitter::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
	const vk::ImageSubresourceRange subresourceRange{
	    vk::ImageAspectFlagBits::eColor,
	    0,
	    1,
	    0,
	    1};

	vk::AccessFlags srcAccessMask{};
	vk::AccessFlags dstAccessMask{};

	vk::PipelineStageFlags sourceStage{};
	vk::PipelineStageFlags destinationStage{};

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

std::vector<vk::CommandBuffer> vkx::CommandSubmitter::allocateDrawCommands(std::uint32_t amount, vk::CommandBufferLevel level) const {
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
	    *commandPool,
	    level,
	    amount * vkx::MAX_FRAMES_IN_FLIGHT};

	return logicalDevice.allocateCommandBuffers(commandBufferAllocateInfo);
}

vk::Result vkx::CommandSubmitter::presentToSwapchain(const vkx::Swapchain& swapchain, std::uint32_t imageIndex, const vkx::SyncObjects& syncObjects) const {
	const vk::PresentInfoKHR presentInfo{
	    *syncObjects.renderFinishedSemaphore,
	    *swapchain.swapchain,
	    imageIndex};

	return presentQueue.presentKHR(presentInfo);
}