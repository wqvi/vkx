#pragma once

#include <vkx/renderer/allocator.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/types.hpp>
#include <vkx/renderer/sync_objects.hpp>
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

class VulkanInstance {
	friend class pipeline::GraphicsPipeline;
	friend class CommandSubmitter;
	friend class Texture;
	friend struct DrawInfo; // fix this!

private:
	SDL_Window* window = nullptr;
	vk::Instance instance;
	vk::SurfaceKHR surface;
	vk::PhysicalDevice physicalDevice;
	vk::Device logicalDevice;
	float maxSamplerAnisotropy = 0;
	vk::Format depthFormat;
	VmaAllocator allocator;
	vk::RenderPass clearRenderPass;

	struct Swapchain {
		SDL_Window* window;
		vk::SurfaceKHR surface;
		vk::PhysicalDevice physicalDevice;
		vk::Device logicalDevice;
		VmaAllocator allocator;
		vk::RenderPass renderPass;
		vk::SwapchainKHR swapchain;
		vk::Extent2D imageExtent;
		std::vector<vk::ImageView> imageViews;
		vk::Format depthFormat;
		vkx::Image depthImage;
		vk::ImageView depthImageView;
		std::vector<vk::Framebuffer> framebuffers;

		void recreate();

		void destroy();

		vk::ResultValue<std::uint32_t> acquireNextImage(const vkx::SyncObjects& syncObjects) const;
	} swapchain;

public:
	VulkanInstance() = default;

	explicit VulkanInstance(SDL_Window* window);

	[[nodiscard]] vk::RenderPass createRenderPass(vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined, vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR) const;

	[[nodiscard]] vk::Format findSupportedFormat(vk::ImageTiling tiling, vk::FormatFeatureFlags features, const std::vector<vk::Format>& candidates) const;

	[[nodiscard]] vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

	[[nodiscard]] vkx::CommandSubmitter createCommandSubmitter() const;

	[[nodiscard]] vkx::pipeline::GraphicsPipeline createGraphicsPipeline(const vkx::pipeline::GraphicsPipelineInformation& information) const;

	[[nodiscard]] std::vector<vkx::SyncObjects> createSyncObjects() const;

	[[nodiscard]] vk::Sampler createTextureSampler() const;

	void waitIdle() const;

	void destroy();

	[[nodiscard]] vkx::Buffer allocateBuffer(std::size_t memorySize,
						 vk::BufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
						 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] vkx::Image allocateImage(vk::Extent2D extent,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
					       VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] std::vector<vkx::UniformBuffer> allocateUniformBuffers(std::size_t memorySize, std::size_t amount) const;

private:
	[[nodiscard]] std::uint32_t ratePhysicalDevice(vk::PhysicalDevice physicalDevice) const;
};
} // namespace vkx
