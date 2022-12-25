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
	std::vector<VkBuffer> vertexBuffers{};
	std::vector<VkBuffer> indexBuffers{};
	std::vector<std::uint32_t> indexCount{};
};

class CommandSubmitter {
private:
	vk::Device device = nullptr;
	vk::CommandPool commandPool = nullptr;
	vk::Queue graphicsQueue = nullptr;
	vk::Queue presentQueue = nullptr;

public:
	CommandSubmitter() = default;

	explicit CommandSubmitter(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);

	void submitImmediately(const std::function<void(VkCommandBuffer)>& command) const;

	void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const;

	void copyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height) const;

	std::vector<VkCommandBuffer> allocateDrawCommands(std::uint32_t amount) const;

	std::vector<VkCommandBuffer> allocateSecondaryDrawCommands(std::uint32_t amount) const;

	void recordPrimaryDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const DrawInfo& drawInfo) const;

	void recordSecondaryDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const VkCommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const;

	void submitDrawCommands(const VkCommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const;

	VkResult presentToSwapchain(const Swapchain& swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const;

	void destroy() const;
};
} // namespace vkx
