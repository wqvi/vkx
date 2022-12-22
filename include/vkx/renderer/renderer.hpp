#pragma once

#include "core/queue_config.hpp"
#include "core/swapchain_info.hpp"
#include "core/sync_objects.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/window.hpp>

namespace vkx {
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

class VulkanAllocator {
private:
	VmaAllocator allocator = nullptr;

public:
	VulkanAllocator() = default;

	explicit VulkanAllocator(VkInstance instance,
				 VkPhysicalDevice physicalDevice,
				 VkDevice logicalDevice);

	VulkanAllocator(const VulkanAllocator& other) = delete;

	VulkanAllocator(VulkanAllocator&& other) noexcept;

	~VulkanAllocator();

	VulkanAllocator& operator=(const VulkanAllocator& other) = delete;

	VulkanAllocator& operator=(VulkanAllocator&& other) noexcept;

	explicit operator VmaAllocator() const;

	[[nodiscard]] vkx::Buffer allocateBuffer(const void* data,
						 std::size_t memorySize,
						 VkBufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
						 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;
};

class VulkanRenderPass {
private:
	VkDevice logicalDevice = nullptr;
	VkRenderPass renderPass = nullptr;

public:
	VulkanRenderPass() = default;

	explicit VulkanRenderPass(VkDevice logicalDevice,
				  VkFormat depthFormat,
				  VkFormat colorFormat,
				  VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				  VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	VulkanRenderPass(const VulkanRenderPass& other) = delete;

	VulkanRenderPass(VulkanRenderPass&& other) noexcept;

	~VulkanRenderPass();

	VulkanRenderPass& operator=(const VulkanRenderPass& other) = delete;

	VulkanRenderPass& operator=(VulkanRenderPass&& other) noexcept;

	explicit operator VkRenderPass() const;
};

class VulkanDevice {
private:
	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice logicalDevice = nullptr;
	float maxSamplerAnisotropy = 0;

public:
	VulkanDevice() = default;

	explicit VulkanDevice(VkInstance instance,
			      VkSurfaceKHR surface,
			      VkPhysicalDevice physicalDevice);

	VulkanDevice(const VulkanDevice& other) = delete;

	VulkanDevice(VulkanDevice&& other) noexcept;

	~VulkanDevice();

	VulkanDevice& operator=(const VulkanDevice& other) = delete;

	VulkanDevice& operator=(VulkanDevice&& other) noexcept;

	explicit operator VkDevice() const;

	[[nodiscard]] vkx::QueueConfig getQueueConfig() const;

	[[nodiscard]] vkx::SwapchainInfo getSwapchainInfo() const;

	[[nodiscard]] vkx::VulkanRenderPass createRenderPass(VkFormat colorFormat,
							     VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
							     VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
							     VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) const;

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
	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;

public:
	VulkanInstance() = default;

	explicit VulkanInstance(const vkx::Window& window);

	VulkanInstance(const VulkanInstance& other) = delete;

	VulkanInstance(VulkanInstance&& other) noexcept;

	~VulkanInstance();

	VulkanInstance& operator=(const VulkanInstance& other) = delete;

	VulkanInstance& operator=(VulkanInstance&& other) noexcept;

	VulkanDevice createDevice() const;

private:
	[[nodiscard]] std::uint32_t ratePhysicalDevice(VkPhysicalDevice physicalDevice) const;
};

class Buffer {
private:
	VmaAllocator allocator = nullptr;
	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo{};

public:
	Buffer() = default;

	explicit Buffer(VmaAllocator allocator,
			const void* data,
			std::size_t memorySize,
			VkBufferUsageFlags bufferFlags,
			VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

	Buffer(const Buffer& other) = delete;

	Buffer(Buffer&& other) noexcept;

	~Buffer();

	Buffer& operator=(const Buffer& other) = delete;

	Buffer& operator=(Buffer&& other) noexcept;

	explicit operator VkBuffer() const;

	void mapMemory(const void* data);
};

class Mesh {
private:
	vkx::Buffer vertexBuffer{};
	vkx::Buffer indexBuffer{};
	std::vector<vkx::Vertex> vertices{};
	std::vector<std::uint32_t> indices{};
	std::size_t activeIndexCount = 0;

public:
	Mesh() = default;

	explicit Mesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanAllocator& allocator);

	[[nodiscard]] const vkx::Buffer& getVertexBuffer() const;

	[[nodiscard]] const vkx::Buffer& getIndexBuffer() const;

	[[nodiscard]] std::size_t getActiveIndexCount() const;
};
} // namespace vkx
