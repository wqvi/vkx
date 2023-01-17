#pragma once

#include <vkx/renderer/model.hpp>
#include <vkx/renderer/graphics.hpp>
#include <vkx/renderer/swapchain.hpp>

namespace vkx {
struct DrawInfo {
	const std::uint32_t imageIndex = 0;
	const std::uint32_t currentFrame = 0;
	const vkx::Swapchain* swapchain{};
	const vkx::GraphicsPipeline* graphicsPipeline{};
	const vkx::VulkanRenderPass* renderPass{};
	const std::vector<vkx::Mesh>& meshes;
};

class CommandSubmitter {
private:
	vk::Device logicalDevice{};
	vk::UniqueCommandPool commandPool{};
	vk::Queue graphicsQueue{};
	vk::Queue presentQueue{};

public:
	CommandSubmitter() = default;

	explicit CommandSubmitter(vk::PhysicalDevice physicalDevice,
				  vk::Device logicalDevice,
				  vk::SurfaceKHR surface);

	template <class T>
	void submitImmediately(T command) const {
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
		    *commandPool,
		    vk::CommandBufferLevel::ePrimary,
		    1};

		const vk::UniqueCommandBuffer commandBuffer = std::move(logicalDevice.allocateCommandBuffersUnique(commandBufferAllocateInfo)[0]);

		const vk::CommandBufferBeginInfo commandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

		commandBuffer->begin(commandBufferBeginInfo);

		command(*commandBuffer);

		commandBuffer->end();

		const vk::SubmitInfo submitInfo{{}, {}, *commandBuffer};

		graphicsQueue.submit(submitInfo);
		graphicsQueue.waitIdle();
	}

	void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const;

	std::vector<vk::CommandBuffer> allocateDrawCommands(std::uint32_t amount, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;

	template <class T>
	void recordPrimaryDrawCommands(T begin, std::uint32_t size, const vkx::DrawInfo& drawInfo) const {
		const auto extent = drawInfo.swapchain->imageExtent;
		const auto framebuffer = *drawInfo.swapchain->framebuffers[drawInfo.imageIndex];

		const vk::CommandBufferBeginInfo commandBufferBeginInfo{};

		const vk::Rect2D renderArea{
		    {0, 0},
		    extent};

		constexpr std::array clearColor{0.0f, 0.0f, 0.0f, 1.0f};
		constexpr vk::ClearDepthStencilValue clearDepthStencil{1.0f, 0};

		const std::array clearValues{vk::ClearValue{clearColor}, vk::ClearValue{clearDepthStencil}};

		const vk::RenderPassBeginInfo renderPassBeginInfo{
		    *drawInfo.renderPass->renderPass,
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
			const vk::CommandBuffer commandBuffer = begin[i];
			const auto& mesh = drawInfo.meshes[i];

			commandBuffer.reset();

			commandBuffer.begin(commandBufferBeginInfo);

			commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline->pipeline);

			commandBuffer.setViewport(0, viewport);

			commandBuffer.setScissor(0, renderArea);

			commandBuffer.bindVertexBuffers(0, static_cast<vk::Buffer>(mesh.vertexBuffer), {0});

			commandBuffer.bindIndexBuffer(static_cast<vk::Buffer>(mesh.indexBuffer), 0, vk::IndexType::eUint32);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline->pipelineLayout, 0, drawInfo.graphicsPipeline->descriptorSets[drawInfo.currentFrame], {});

			commandBuffer.drawIndexed(static_cast<std::uint32_t>(mesh.activeIndexCount), 1, 0, 0, 0);

			commandBuffer.endRenderPass();

			commandBuffer.end();
		}
	}

	template <class T>
	void recordSecondaryDrawCommands(T begin, std::uint32_t size, T secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const {
		const auto extent = drawInfo.swapchain->imageExtent;
		const auto framebuffer = *drawInfo.swapchain->framebuffers[drawInfo.imageIndex];

		const vk::CommandBufferBeginInfo commandBufferBeginInfo{};

		const vk::CommandBufferInheritanceInfo secondaryCommandBufferInheritanceInfo{
		    *drawInfo.renderPass->renderPass,
		    0,
		    framebuffer};

		const vk::CommandBufferBeginInfo secondaryCommandBufferBeginInfo{
		    vk::CommandBufferUsageFlagBits::eRenderPassContinue,
		    &secondaryCommandBufferInheritanceInfo};

		const vk::Rect2D renderArea{
		    {0, 0},
		    extent};

		constexpr std::array clearColor{0.0f, 0.0f, 0.0f, 1.0f};
		constexpr vk::ClearDepthStencilValue clearDepthStencil{1.0f, 0};

		const std::array clearValues{vk::ClearValue{clearColor}, vk::ClearValue{clearDepthStencil}};

		const vk::RenderPassBeginInfo renderPassBeginInfo{
			*drawInfo.renderPass->renderPass,
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
			const vk::CommandBuffer commandBuffer = begin[i];

			commandBuffer.reset();

			commandBuffer.begin(commandBufferBeginInfo);

			commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eSecondaryCommandBuffers);

			for (std::uint32_t j = 0; j < secondarySize; j++) {
				const vk::CommandBuffer secondaryCommandBuffer = secondaryBegin[j];
				const auto& mesh = drawInfo.meshes[j];

				secondaryCommandBuffer.begin(secondaryCommandBufferBeginInfo);

				secondaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline->pipeline);

				secondaryCommandBuffer.setViewport(0, viewport);

				secondaryCommandBuffer.setScissor(0, renderArea);

				secondaryCommandBuffer.bindVertexBuffers(0, static_cast<vk::Buffer>(mesh.vertexBuffer), {0});

				secondaryCommandBuffer.bindIndexBuffer(static_cast<vk::Buffer>(mesh.indexBuffer), 0, vk::IndexType::eUint32);

				secondaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *drawInfo.graphicsPipeline->pipelineLayout, 0, drawInfo.graphicsPipeline->descriptorSets[drawInfo.currentFrame], {});

				secondaryCommandBuffer.drawIndexed(static_cast<std::uint32_t>(mesh.activeIndexCount), 1, 0, 0, 0);

				secondaryCommandBuffer.end();
			}

			commandBuffer.executeCommands(secondarySize, secondaryBegin);

			commandBuffer.endRenderPass();

			commandBuffer.end();
		}
	}

	template <class T>
	void submitDrawCommands(T begin, std::uint32_t size, const vkx::SyncObjects& syncObjects) const {
		constexpr std::array<vk::PipelineStageFlags, 1> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};

		const vk::SubmitInfo submitInfo{
		    1,
		    &*syncObjects.imageAvailableSemaphore,
		    waitStages.data(),
		    size,
		    begin,
		    1,
		    &*syncObjects.renderFinishedSemaphore};

		graphicsQueue.submit(submitInfo, *syncObjects.inFlightFence);
	}

	vk::Result presentToSwapchain(const vkx::Swapchain& swapchain, std::uint32_t imageIndex, const vkx::SyncObjects& syncObjects) const;
};
} // namespace vkx
