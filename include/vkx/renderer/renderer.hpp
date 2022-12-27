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

[[nodiscard]] VmaAllocation allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

[[nodiscard]] VmaAllocation allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, const void* ptr, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

class VulkanInstance;
class VulkanDevice;
class VulkanRenderPass;
class VulkanAllocator;

class VulkanAllocationDeleter {
private:
	VmaAllocator allocator = nullptr;

public:
	VulkanAllocationDeleter() = default;

	explicit VulkanAllocationDeleter(VmaAllocator allocator);

	void operator()(VmaAllocation allocation) const noexcept;
};

using UniqueVulkanAllocation = std::unique_ptr<std::remove_pointer_t<VmaAllocation>, VulkanAllocationDeleter>;

class Buffer {
private:
	vk::UniqueBuffer buffer;
	vkx::UniqueVulkanAllocation allocation;
	VmaAllocationInfo allocationInfo{};

public:
	Buffer() = default;

	explicit Buffer(vk::UniqueBuffer&& buffer, vkx::UniqueVulkanAllocation&& allocation, VmaAllocationInfo&& allocationInfo);

	explicit operator VkBuffer() const;

	template <class T>
	void mapMemory(const T* data) const {
		std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
	}

	std::size_t size() const;
};

class Image {
private:
	vk::Device logicalDevice = nullptr;
	vk::UniqueImage resourceImage;
	UniqueVulkanAllocation resourceAllocation;

public:
	Image() = default;

	explicit Image(vk::Device logicalDevice, vk::UniqueImage&& image, UniqueVulkanAllocation&& allocation);

	inline VkImageView createTextureImageView(VkDevice device) const {
		return vkx::createImageView(device, *resourceImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	vk::UniqueImageView createView(vk::Format format, vk::ImageAspectFlags aspectFlags) const;
};

class UniformBuffer {
private:
	vk::DescriptorBufferInfo info{};
	vkx::Buffer buffer;

public:
	UniformBuffer() = default;

	UniformBuffer(vkx::Buffer&& buffer)
	    : info(static_cast<VkBuffer>(buffer), 0, buffer.size()),
	      buffer(std::move(buffer)) {}

	template <class T>
	void mapMemory(const T& obj) const {
		buffer.mapMemory(&obj);
	}

	vk::DescriptorBufferInfo createDescriptorBufferInfo() const {
		return {static_cast<VkBuffer>(buffer), 0, buffer.size()};
	}

	const vk::DescriptorBufferInfo* getInfo() const {
		return &info;
	}
};

struct VulkanAllocatorDeleter {
	void operator()(VmaAllocator allocator) const noexcept;
};

using UniqueVulkanAllocator = std::unique_ptr<std::remove_pointer_t<VmaAllocator>, VulkanAllocatorDeleter>;

class VulkanAllocator {
private:
	vk::Device logicalDevice = nullptr;
	UniqueVulkanAllocator allocator;

public:
	VulkanAllocator() = default;

	explicit VulkanAllocator(vk::Instance instance,
				 vk::PhysicalDevice physicalDevice,
				 vk::Device logicalDevice);

	explicit operator VmaAllocator() const;

	template <class T>
	[[nodiscard]] vkx::Buffer allocateBuffer(const T* data,
						 std::size_t memorySize,
						 vk::BufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
						 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const {
		const vk::BufferCreateInfo bufferCreateInfo{{}, memorySize, bufferFlags, vk::SharingMode::eExclusive};

		const VmaAllocationCreateInfo allocationCreateInfo{
		    allocationFlags,
		    memoryUsage,
		    0,
		    0,
		    0,
		    nullptr,
		    nullptr,
		    {}};

		VkBuffer cBuffer = nullptr;
		VmaAllocation cAllocation = nullptr;
		VmaAllocationInfo cAllocationInfo;
		if (vmaCreateBuffer(allocator.get(), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &cBuffer, &cAllocation, &cAllocationInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate GPU buffer.");
		}

		vkx::Buffer buffer{vk::UniqueBuffer(cBuffer, logicalDevice), UniqueVulkanAllocation(cAllocation, VulkanAllocationDeleter{allocator.get()}), std::move(cAllocationInfo)};
		buffer.mapMemory(data);
		return buffer;
	}

	[[nodiscard]] vkx::Buffer allocateBuffer(std::size_t memorySize,
						 vk::BufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
						 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const {
		const vk::BufferCreateInfo bufferCreateInfo{{}, memorySize, bufferFlags, vk::SharingMode::eExclusive};

		const VmaAllocationCreateInfo allocationCreateInfo{
		    allocationFlags,
		    memoryUsage,
		    0,
		    0,
		    0,
		    nullptr,
		    nullptr,
		    {}};

		VkBuffer cBuffer = nullptr;
		VmaAllocation cAllocation = nullptr;
		VmaAllocationInfo cAllocationInfo;
		if (vmaCreateBuffer(allocator.get(), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &cBuffer, &cAllocation, &cAllocationInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate GPU buffer.");
		}

		return vkx::Buffer{vk::UniqueBuffer(cBuffer, logicalDevice), UniqueVulkanAllocation(cAllocation, VulkanAllocationDeleter{allocator.get()}), std::move(cAllocationInfo)};
	}

	[[nodiscard]] vkx::Image allocateImage(vk::Extent2D extent,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
					       VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] vkx::Image allocateImage(const vkx::CommandSubmitter& commandSubmitter,
					       const std::string& file,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
					       VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] vkx::UniformBuffer allocateUniformBuffer(std::size_t memorySize) const;

	[[nodiscard]] std::vector<vkx::UniformBuffer> allocateUniformBuffers(std::size_t memorySize, std::size_t amount) const;
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
