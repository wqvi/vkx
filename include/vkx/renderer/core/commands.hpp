#pragma once

#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/sync_objects.hpp>

namespace vkx {
struct DrawInfo {
	std::uint32_t imageIndex = 0;
	std::uint32_t currentFrame = 0;
	vkx::Swapchain* swapchain{};
	const vkx::GraphicsPipeline* graphicsPipeline{};
	VkRenderPass renderPass{};
	std::vector<vkx::Mesh>& meshes;
};

class CommandSubmitter {
private:
	vk::Device device = nullptr;
	vk::UniqueCommandPool commandPool;
	vk::Queue graphicsQueue = nullptr;
	vk::Queue presentQueue = nullptr;

public:
	CommandSubmitter() = default;

	explicit CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);

	template <class T>
	void submitImmediately(T command) const {
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
		    *commandPool,
		    vk::CommandBufferLevel::ePrimary,
		    1};

		const auto commandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferAllocateInfo)[0]);

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

	std::vector<vk::CommandBuffer> allocateDrawCommands(std::uint32_t amount) const;

	std::vector<vk::CommandBuffer> allocateSecondaryDrawCommands(std::uint32_t amount) const;

	void recordPrimaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const;

	void recordSecondaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vk::CommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const;

	void submitDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const;

	vk::Result presentToSwapchain(const Swapchain& swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const;
};
} // namespace vkx
