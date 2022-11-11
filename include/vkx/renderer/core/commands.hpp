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
	CommandSubmitter() = default;

	explicit CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);

	void submitImmediately(const std::function<void(vk::CommandBuffer)>& command) const;

	void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const;

	std::vector<VkCommandBuffer> allocateDrawCommands(std::uint32_t amount) const;

	std::vector<VkCommandBuffer> allocateSecondaryDrawCommands(std::uint32_t amount) const;

	void recordPrimaryDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const;

	void recordSecondaryDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const VkCommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const;

	void submitDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const;

	vk::Result presentToSwapchain(const Swapchain& swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const;

	void destroy() const;

private:
	VkDevice device = nullptr;
	VkCommandPool commandPool = nullptr;
	VkQueue graphicsQueue = nullptr;
	VkQueue presentQueue = nullptr;
};
} // namespace vkx
