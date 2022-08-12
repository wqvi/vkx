#pragma once

#include "queue_config.hpp"
#include "renderer_types.hpp"
#include "vertex.hpp"
#include "vk_mem_alloc.h"
#include "vkx/renderer/core/commands.hpp"
#include "vkx/renderer/core/pipeline.hpp"
#include "vkx/renderer/uniform_buffer.hpp"
#include <vulkan/vulkan_handles.hpp>
#include <vkx/renderer/core/swapchain.hpp>

namespace vkx {
class Device {
public:
	Device() = default;

	explicit Device(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

	explicit operator const vk::PhysicalDevice&() const;

	explicit operator const vk::Device&() const;

	const vk::Device& operator*() const;

	const vk::Device* operator->() const;

	[[nodiscard]] std::vector<DrawCommand> createDrawCommands(std::uint32_t size) const;

	[[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

	[[nodiscard]] inline vk::Format findDepthFormat() const {
		return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	[[nodiscard]] vk::UniqueImageView createImageViewUnique(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

	// void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const;

	// void copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const;

	// void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	// void submit(const std::vector<vk::CommandBuffer>& commandBuffers, vk::Semaphore waitSemaphore, vk::Semaphore signalSemaphore, vk::Fence flightFence) const;

	// [[nodiscard]] vk::Result present(vk::SwapchainKHR swapchain, std::uint32_t imageIndex, vk::Semaphore signalSemaphores) const;

	[[nodiscard]] vk::UniqueImageView createTextureImageViewUnique(vk::Image image) const;

	[[nodiscard]] vk::UniqueSampler createTextureSamplerUnique() const;

	std::shared_ptr<vkx::Allocator> createAllocator() const;

	std::shared_ptr<vkx::Swapchain> createSwapchain(SDL_Window* window, const std::shared_ptr<vkx::Allocator>& allocator) const;

	vk::UniqueRenderPass createRenderPass(vk::Format format, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) const;

	std::shared_ptr<vkx::GraphicsPipeline> createGraphicsPipeline(const vk::Extent2D& extent, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout) const;

	std::shared_ptr<vkx::CommandSubmitter> createCommandSubmitter() const;

private:
	vk::Instance instance{};
	vk::SurfaceKHR surface{};
	vk::PhysicalDevice physicalDevice{};
	vk::UniqueDevice device{};
	float maxSamplerAnisotropy = 0.0f;
};

struct DrawInfo {
	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;
	vk::Extent2D extent;
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout graphicsPipelineLayout;
	vk::DescriptorSet descriptorSet;
	vk::Buffer vertexBuffer;
	vk::Buffer indexBuffer;
	std::uint32_t indexCount;
};

class CommandSubmitter {
	using iter = std::vector<vk::CommandBuffer>::const_iterator;

public:
	explicit CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);

	void submitImmediately(const std::function<void(vk::CommandBuffer)>& command) const;

	void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const;

	std::vector<vk::CommandBuffer> allocateDrawCommands(std::uint32_t amount) const;

	void recordDrawCommands(iter begin, iter end, const DrawInfo& drawInfo) const;

	void submitDrawCommands(iter begin, iter end, const SyncObjects& syncObjects) const;

	vk::Result presentToSwapchain(vk::SwapchainKHR swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const;

private:
	vk::Device device{};
	vk::UniqueCommandPool commandPool{};
	vk::Queue graphicsQueue{};
	vk::Queue presentQueue{};
};
} // namespace vkx