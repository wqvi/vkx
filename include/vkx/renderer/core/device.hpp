#pragma once

#include <vkx/renderer/core/allocator.hpp>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/pipeline.hpp>
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

	[[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

	[[nodiscard]] inline vk::Format findDepthFormat() const {
		return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	[[nodiscard]] vk::UniqueImageView createImageViewUnique(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

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
} // namespace vkx