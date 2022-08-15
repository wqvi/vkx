#pragma once

#include <vkx/renderer/core/sync_objects.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
struct DrawInfo {
	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;
	vk::Extent2D extent;
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout graphicsPipelineLayout;
	vk::DescriptorSet descriptorSet;
	std::vector<vk::Buffer> vertexBuffers;
	std::vector<vk::Buffer> indexBuffers;
	std::vector<std::uint32_t> indexCount;
};

class CommandSubmitter {
	using iter = std::vector<vk::CommandBuffer>::const_iterator;

public:
	explicit CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);

	void submitImmediately(const std::function<void(vk::CommandBuffer)>& command) const;

	void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const;

	std::vector<vk::CommandBuffer> allocateDrawCommands(std::uint32_t amount) const;

	std::vector<vk::CommandBuffer> allocateSecondaryDrawCommands(std::uint32_t amount) const;

	void recordDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const vk::CommandBuffer* secondaryBegin, std::uint32_t secondarySize, const DrawInfo& drawInfo) const;

	void submitDrawCommands(const vk::CommandBuffer* begin, std::uint32_t size, const SyncObjects& syncObjects) const;

	vk::Result presentToSwapchain(vk::SwapchainKHR swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const;

private:
	vk::Device device{};
	vk::UniqueCommandPool commandPool{};
	vk::Queue graphicsQueue{};
	vk::Queue presentQueue{};
};
} // namespace vkx