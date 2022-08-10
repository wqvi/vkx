#pragma once

#include "queue_config.hpp"
#include "renderer_types.hpp"
#include "vertex.hpp"
#include "vk_mem_alloc.h"
#include "vkx/renderer/core/pipeline.hpp"
#include <SDL2/SDL_log.h>
#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace vkx {
template <class T>
class Allocation;

class Allocator;

class Allocator {
public:
	Allocator() = default;

	Allocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);

	~Allocator();

	[[nodiscard]] VmaAllocator getAllocator() const noexcept;

	[[nodiscard]] std::shared_ptr<Allocation<vk::Image>> allocateImage(std::uint32_t width, std::uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] std::shared_ptr<Allocation<vk::Buffer>> allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	template <class T>
	std::shared_ptr<Allocation<vk::Buffer>> allocateBuffer(const std::vector<T>& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	template <class T, std::size_t size>
	std::shared_ptr<Allocation<vk::Buffer>> allocateBuffer(const std::array<T, size>& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	template <class T>
	std::shared_ptr<Allocation<vk::Buffer>> allocateBuffer(const T& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

private:
	VmaAllocator allocator = nullptr;

	static VmaAllocationCreateInfo createAllocationInfo(VmaAllocationCreateFlags flags = 0, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO, VmaPool pool = nullptr);
};

template <class T>
struct Allocation {
	const T object = {};
	const VmaAllocation allocation = nullptr;
	const VmaAllocationInfo allocationInfo = {};
	const VmaAllocator allocator = nullptr;

	Allocation() = default;

	constexpr Allocation(const T object, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo, VmaAllocator allocator)
	    : object(object), allocation(allocation), allocationInfo(allocationInfo), allocator(allocator) {}

	~Allocation();

	template <class K>
	void mapMemory(const std::vector<K>& memory) const {
#ifdef DEBUG
		// Ensure memory size in bytes is equal to the mapped memory's size in bytes
		const auto size = memory.size() * sizeof(K);
		if (size < allocationInfo.size) {
			throw std::invalid_argument("Provided memory is too small to be mapped.");
		}
		if (size > allocationInfo.size) {
			throw std::invalid_argument("Provided memory is too large to be mapped.");
		}
#endif
		std::memcpy(memory.data(), allocationInfo.pMappedData, allocationInfo.size);
	}
};

template <>
inline Allocation<vk::Image>::~Allocation() {
	vmaDestroyImage(allocator, static_cast<VkImage>(object), allocation);
#ifdef DEBUG
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Freed image resource allocation.");
#endif
}

template <>
inline Allocation<vk::Buffer>::~Allocation() {
	vmaDestroyBuffer(allocator, static_cast<VkBuffer>(object), allocation);
#ifdef DEBUG
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Freed buffer resource allocation.");
#endif
}

template <class T>
std::shared_ptr<Allocation<vk::Buffer>> Allocator::allocateBuffer(const std::vector<T>& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo({}, data.size() * sizeof(T), bufferUsage, vk::SharingMode::eExclusive);

	const VmaAllocationCreateInfo allocationCreateInfo = createAllocationInfo(flags, memoryUsage);

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	std::memcpy(allocationInfo.pMappedData, data.data(), allocationInfo.size);

	return std::make_shared<Allocation<vk::Buffer>>(vk::Buffer(buffer), allocation, allocationInfo, allocator);
}

template <class T, std::size_t size>
std::shared_ptr<Allocation<vk::Buffer>> Allocator::allocateBuffer(const std::array<T, size>& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo({}, sizeof(T) * size, bufferUsage, vk::SharingMode::eExclusive);

	const VmaAllocationCreateInfo allocationCreateInfo = createAllocationInfo(flags, memoryUsage);

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	std::memcpy(allocationInfo.pMappedData, data.data(), allocationInfo.size);

	return std::make_shared<Allocation<vk::Buffer>>(vk::Buffer(buffer), allocation, allocationInfo, allocator);
}

template <class T>
std::shared_ptr<Allocation<vk::Buffer>> Allocator::allocateBuffer(const T& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo({}, sizeof(T), bufferUsage, vk::SharingMode::eExclusive);

	const VmaAllocationCreateInfo allocationCreateInfo = createAllocationInfo(flags, memoryUsage);

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	std::memcpy(allocationInfo.pMappedData, &data, allocationInfo.size);

	return std::make_shared<Allocation<vk::Buffer>>(vk::Buffer(buffer), allocation, allocationInfo, allocator);
}

class Device {
public:
	Device() = default;

	explicit Device(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

	explicit operator const vk::PhysicalDevice&() const;

	explicit operator const vk::Device&() const;

	explicit operator const vk::CommandPool&() const;

	explicit operator const vk::UniqueCommandPool&() const;

	const vk::Device& operator*() const;

	const vk::Device* operator->() const;

	[[nodiscard]] std::vector<DrawCommand> createDrawCommands(std::uint32_t size) const;

	[[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

	[[nodiscard]] inline vk::Format findDepthFormat() const {
		return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	[[nodiscard]] vk::UniqueImageView createImageViewUnique(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const;

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const;

	void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	void submit(const std::vector<vk::CommandBuffer>& commandBuffers, vk::Semaphore waitSemaphore, vk::Semaphore signalSemaphore, vk::Fence flightFence) const;

	[[nodiscard]] vk::Result present(vk::SwapchainKHR swapchain, std::uint32_t imageIndex, vk::Semaphore signalSemaphores) const;

	[[nodiscard]] vk::UniqueImageView createTextureImageViewUnique(vk::Image image) const;

	[[nodiscard]] vk::UniqueSampler createTextureSamplerUnique() const;

	std::shared_ptr<vkx::Allocator> createAllocator() const;

	std::shared_ptr<vkx::Swapchain> createSwapchain(SDL_Window* window, const std::shared_ptr<vkx::Allocator>& allocator) const;

	vk::UniqueRenderPass createRenderPass(vk::Format format, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) const;

	std::shared_ptr<vkx::GraphicsPipeline> createGraphicsPipeline(const vk::Extent2D& extent, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout) const;

private:
	vk::Instance instance{};
	vk::SurfaceKHR surface{};
	vk::PhysicalDevice physicalDevice{};
	vk::UniqueDevice device{};
	vk::UniqueCommandPool commandPool{};
	vk::PhysicalDeviceProperties properties{};
	Queues queues{};
};
} // namespace vkx