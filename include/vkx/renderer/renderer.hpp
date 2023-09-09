#pragma once

#include <vkx/renderer/memory/allocator.hpp>
#include <vkx/renderer/types.hpp>
#include <vkx/window.hpp>

namespace vkx {
static constexpr std::uint32_t VARIANT = UINT32_C(0);
static constexpr std::uint32_t MAJOR = UINT32_C(0);
static constexpr std::uint32_t MINOR = UINT32_C(1);
static constexpr std::uint32_t PATCH = UINT32_C(0);
static constexpr auto VERSION = VK_MAKE_API_VERSION(VARIANT, MAJOR, MINOR, PATCH);

template <class ObjectType, class Function, class Predicate, class... Parameters>
constexpr std::enable_if_t<std::is_same_v<std::invoke_result_t<Function, Parameters..., ObjectType*>, VkResult>, ObjectType> getObject(const char* errorMessage, Function function, Predicate predicate, Parameters... param) {
	ObjectType object{};
	const auto result = function(param..., &object);
	if (predicate(result)) {
		throw std::runtime_error(errorMessage);
	}

	return object;
}

template <class ObjectType, class Function, class... Parameters>
constexpr std::enable_if_t<std::is_same_v<std::invoke_result_t<Function, Parameters..., ObjectType*>, void>, ObjectType> getObject(Function function, Parameters... param) {
	ObjectType object{};
	function(param..., &object);

	return object;
}

template <class ArrayType, class Function, class Predicate, class... Parameters>
constexpr std::enable_if_t<std::is_same_v<std::invoke_result_t<Function, Parameters..., std::uint32_t*, ArrayType*>, SDL_bool> || std::is_same_v<std::invoke_result_t<Function, Parameters..., std::uint32_t*, ArrayType*>, VkResult>, std::vector<ArrayType>> getArray(const char* errorMessage, Function function, Predicate predicate, Parameters... param) {
	std::uint32_t count = 0;
	auto result = function(param..., &count, nullptr);
	if (predicate(result)) {
		throw std::runtime_error(errorMessage);
	}

	std::vector<ArrayType> array{count};
	result = function(param..., &count, array.data());
	if (predicate(result)) {
		throw std::runtime_error(errorMessage);
	}

	return array;
}

template <class ArrayType, class Function, class... Parameters>
constexpr std::enable_if_t<std::is_same_v<std::invoke_result_t<Function, Parameters..., std::uint32_t*, ArrayType*>, void>, std::vector<ArrayType>> getArray(Function function, Parameters... param) {
	std::uint32_t count = 0;
	function(param..., &count, nullptr);

	std::vector<ArrayType> array{count};
	function(param..., &count, array.data());

	return array;
}

template <class ObjectType, class Function, class Predicate, class... Parameters>
constexpr auto create(Function function, Predicate predicate, Parameters... param) {
	ObjectType object{};
	auto result = function(param..., &object);
	predicate(result);

	return object;
}

template <class T>
constexpr auto posMod(T a, T b) {
	const auto value = std::fmod(a, b);
	if ((value < 0.0f && b > 0.0f) || (value > 0.0f && b < 0.0f)) {
		return value + b;
	}
	return value + 0.0f;
}

class VulkanRenderPass {
	friend class VulkanInstance;
	friend class CommandSubmitter;
	friend class Swapchain;

private:
	vk::UniqueRenderPass renderPass;

public:
	VulkanRenderPass() = default;

	explicit VulkanRenderPass(vk::Device logicalDevice,
				  vk::Format depthFormat,
				  vk::Format colorFormat,
				  vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
				  vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
				  vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR);
};

class VulkanInstance {
	friend class Swapchain;

private:
	SDL_Window* window = nullptr;
	vk::UniqueInstance instance{};
	vk::UniqueSurfaceKHR surface{};
	vk::PhysicalDevice physicalDevice{};
	vk::UniqueDevice logicalDevice{};
	float maxSamplerAnisotropy = 0;

public:
	VulkanInstance() = default;

	explicit VulkanInstance(const vkx::Window& window);

	[[nodiscard]] vkx::QueueConfig getQueueConfig() const;

	[[nodiscard]] vkx::SwapchainInfo getSwapchainInfo(const vkx::Window& window) const;

	[[nodiscard]] vkx::VulkanRenderPass createRenderPass(vk::Format colorFormat,
							     vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
							     vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
							     vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR) const;

	[[nodiscard]] vkx::VulkanAllocator createAllocator() const;

	[[nodiscard]] vk::Format findSupportedFormat(vk::ImageTiling tiling, vk::FormatFeatureFlags features, const std::vector<vk::Format>& candidates) const;

	[[nodiscard]] inline auto findDepthFormat() const {
		return findSupportedFormat(vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint});
	}

	[[nodiscard]] vk::UniqueImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

	[[nodiscard]] vkx::Swapchain createSwapchain(const vkx::VulkanAllocator& allocator, const vkx::VulkanRenderPass& renderPass, const vkx::Window& window) const;

	[[nodiscard]] vkx::CommandSubmitter createCommandSubmitter() const;

	[[nodiscard]] vkx::pipeline::GraphicsPipeline createGraphicsPipeline(const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, const vkx::pipeline::GraphicsPipelineInformation& information) const;

	[[nodiscard]] vkx::pipeline::ComputePipeline createComputePipeline(const vkx::pipeline::ComputePipelineInformation& information) const;

	[[nodiscard]] std::vector<vkx::SyncObjects> createSyncObjects() const;

	[[nodiscard]] vk::UniqueSampler createTextureSampler() const;

	void waitIdle() const;


private:
	[[nodiscard]] std::uint32_t ratePhysicalDevice(vk::PhysicalDevice physicalDevice) const;
};
} // namespace vkx
