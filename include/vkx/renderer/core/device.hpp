#pragma once

#include <vkx/renderer/core/allocator.hpp>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/core/swapchain.hpp>

namespace vkx {
class Device {
	friend class QueueConfig;
	friend class SwapchainInfo;
public:
	Device() = default;

	explicit Device(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

	Device(const Device&) = delete;

	Device(Device&&) noexcept = default;

	Device& operator=(const Device&) = delete;

	Device& operator=(Device&&) noexcept = default;

	const vk::Device* operator->() const;

	[[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

	[[nodiscard]] inline vk::Format findDepthFormat() const {
		return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	[[nodiscard]] vk::UniqueImageView createImageViewUnique(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

	[[nodiscard]] vk::UniqueImageView createTextureImageViewUnique(vk::Image image) const;

	[[nodiscard]] vk::UniqueSampler createTextureSamplerUnique() const;

	vkx::Allocator createAllocator() const;

	vkx::Swapchain createSwapchain(SDL_Window* window, vk::RenderPass renderPass, const vkx::Allocator& allocator) const;

	vk::UniqueRenderPass createRenderPass(vk::Format format, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined, vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) const;

	std::shared_ptr<vkx::GraphicsPipeline> createGraphicsPipeline(vk::RenderPass renderPass, const Allocator& allocator, const vkx::GraphicsPipelineInformation& info) const;

	vkx::CommandSubmitter createCommandSubmitter() const;

	std::vector<SyncObjects> createSyncObjects() const;

private:
	vk::Instance instance{};
	vk::SurfaceKHR surface{};
	vk::PhysicalDevice physicalDevice{};
	vk::UniqueDevice device{};
	float maxSamplerAnisotropy = 0.0f;
};
} // namespace vkx
