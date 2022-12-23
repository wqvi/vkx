#pragma once

#include "core/queue_config.hpp"
#include "core/swapchain_info.hpp"
#include "core/sync_objects.hpp"
#include <vkx/renderer/core/pipeline.hpp>
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

[[nodiscard]] VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

[[nodiscard]] VkSampler createTextureSampler(VkDevice device, float samplerAnisotropy);

[[nodiscard]] std::vector<vkx::SyncObjects> createSyncObjects(VkDevice device);

[[nodiscard]] VmaAllocation allocateImage(VmaAllocationInfo* allocationInfo, VkImage* image, VmaAllocator allocator, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

[[nodiscard]] VmaAllocation allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

[[nodiscard]] VmaAllocation allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, const void* ptr, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

[[nodiscard]] std::vector<vkx::UniformBuffer> allocateUniformBuffers(VmaAllocator allocator, std::size_t size);

struct VulkanAllocatorDeleter {
	void operator()(VmaAllocator allocator) const noexcept;
};

class VulkanAllocator {
private:
	std::unique_ptr<std::remove_pointer_t<VmaAllocator>, VulkanAllocatorDeleter> allocator;

public:
	VulkanAllocator() = default;

	explicit VulkanAllocator(VkInstance instance,
				 VkPhysicalDevice physicalDevice,
				 VkDevice logicalDevice);

	explicit operator VmaAllocator() const;

	[[nodiscard]] vkx::Buffer allocateBuffer(const void* data,
						 std::size_t memorySize,
						 VkBufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
						 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;
};

class VulkanRenderPass {
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

	explicit operator VkRenderPass() const;
};

class VulkanDevice {
private:
	vk::Instance instance = nullptr;
	vk::SurfaceKHR surface = nullptr;
	vk::PhysicalDevice physicalDevice = nullptr;
	vk::UniqueDevice logicalDevice;
	float maxSamplerAnisotropy = 0;

public:
	VulkanDevice() = default;

	explicit VulkanDevice(vk::Instance instance,
			      vk::SurfaceKHR surface,
			      vk::PhysicalDevice physicalDevice);

	explicit operator VkDevice() const;

	[[nodiscard]] vkx::QueueConfig getQueueConfig() const;

	[[nodiscard]] vkx::SwapchainInfo getSwapchainInfo() const;

	[[nodiscard]] vkx::VulkanRenderPass createRenderPass(vk::Format colorFormat,
							     vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
							     vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
							     vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR) const;

	[[nodiscard]] vkx::VulkanAllocator createAllocator() const;

	[[nodiscard]] VkFormat findSupportedFormat(VkImageTiling tiling, VkFormatFeatureFlags features, const std::vector<VkFormat>& candidates) const;

	[[nodiscard]] inline auto findDepthFormat() const {
		return findSupportedFormat(VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT});
	}

	[[nodiscard]] float getMaxSamplerAnisotropy() const;

	[[nodiscard]] vkx::Swapchain createSwapchain(const vkx::VulkanAllocator& allocator, const vkx::VulkanRenderPass& renderPass, const vkx::Window& window) const;

	[[nodiscard]] vkx::CommandSubmitter createCommandSubmitter() const;

	[[nodiscard]] vkx::GraphicsPipeline createGraphicsPipeline(const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, const vkx::GraphicsPipelineInformation& information) const;

	void waitIdle() const;
};

class VulkanInstance {
private:
	SDL_Window* window = nullptr;
	vk::UniqueInstance instance;
	vk::UniqueSurfaceKHR surface;

public:
	VulkanInstance() = default;

	explicit VulkanInstance(const vkx::Window& window);

	VulkanDevice createDevice() const;

private:
	[[nodiscard]] std::uint32_t ratePhysicalDevice(vk::PhysicalDevice physicalDevice) const;
};

class Buffer {
private:
	VmaAllocator allocator = nullptr;
	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo{};

public:
	Buffer() = default;

	template <class T>
	explicit Buffer(VmaAllocator allocator,
			const T* data,
			std::size_t memorySize,
			VkBufferUsageFlags bufferFlags,
			VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO)
	    : allocator(allocator) {
		const VkBufferCreateInfo bufferCreateInfo{
		    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		    nullptr,
		    0,
		    memorySize,
		    bufferFlags,
		    VK_SHARING_MODE_EXCLUSIVE,
		    0,
		    nullptr};

		const VmaAllocationCreateInfo allocationCreateInfo{
		    allocationFlags,
		    memoryUsage,
		    0,
		    0,
		    0,
		    nullptr,
		    nullptr,
		    {}};

		if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate GPU buffer.");
		}

		if (data != nullptr) {
			std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
		}
	}

	Buffer(const Buffer& other) = delete;

	Buffer(Buffer&& other) noexcept;

	~Buffer();

	Buffer& operator=(const Buffer& other) = delete;

	Buffer& operator=(Buffer&& other) noexcept;

	explicit operator VkBuffer() const;

	void mapMemory(const void* data);
};

struct Mesh {
	vkx::Buffer vertexBuffer{};
	vkx::Buffer indexBuffer{};
	std::vector<vkx::Vertex> vertices{};
	std::vector<std::uint32_t> indices{};
	std::size_t activeIndexCount = 0;

	Mesh() = default;

	explicit Mesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanAllocator& allocator);

	explicit Mesh(std::size_t vertexCount, std::size_t indexCount, const vkx::VulkanAllocator& allocator);
};
} // namespace vkx
