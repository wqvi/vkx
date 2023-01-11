#pragma once

#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/swapchain.hpp>

namespace vkx {
struct DrawInfo {
	const std::uint32_t imageIndex = 0;
	const std::uint32_t currentFrame = 0;
	const vkx::Swapchain* swapchain{};
	const vkx::GraphicsPipeline* graphicsPipeline{};
	const vk::RenderPass renderPass{};
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

	void recordPrimaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vkx::DrawInfo& drawInfo) const;

	void recordSecondaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vk::CommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const;

	void submitDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vkx::SyncObjects& syncObjects) const;

	vk::Result presentToSwapchain(const vkx::Swapchain& swapchain, std::uint32_t imageIndex, const vkx::SyncObjects& syncObjects) const;
};
} // namespace vkx
