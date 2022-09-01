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
	vk::RenderPass renderPass{};
	std::vector<vk::Buffer> vertexBuffers{};
	std::vector<vk::Buffer> indexBuffers{};
	std::vector<std::uint32_t> indexCount{};
};

class CommandSubmitter {
public:
	explicit CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);

	void submitImmediately(const std::function<void(vk::CommandBuffer)>& command) const;

	void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const;

	std::vector<vk::CommandBuffer> allocateDrawCommands(std::uint32_t amount) const;

	std::vector<vk::CommandBuffer> allocateSecondaryDrawCommands(std::uint32_t amount) const;

	void recordPrimaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const;

	void recordSecondaryDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vk::CommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const;

	void submitDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const;

	vk::Result presentToSwapchain(const Swapchain& swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const;

private:
	vk::Device device{};
	vk::UniqueCommandPool commandPool{};
	vk::Queue graphicsQueue{};
	vk::Queue presentQueue{};
};
} // namespace vkx