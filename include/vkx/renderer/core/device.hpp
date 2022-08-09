#pragma once

#include "queue_config.hpp"
#include "renderer_types.hpp"
#include "vertex.hpp"
#include "vk_mem_alloc.h"
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

	template <class T>
	std::shared_ptr<Allocation<vk::Buffer>> allocateBuffer(const T& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

private:
	VmaAllocator allocator = nullptr;
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

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = flags;
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

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

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = flags;
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

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

	explicit Device(const vk::UniqueInstance& instance, const vk::PhysicalDevice& physicalDevice, const vk::UniqueSurfaceKHR& surface);

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

	[[nodiscard]] vk::UniqueImageView createImageViewUnique(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

	void copyBuffer(const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, const vk::DeviceSize& size) const;

	void copyBufferToImage(const vk::Buffer& buffer, const vk::Image& image, std::uint32_t width, std::uint32_t height) const;

	void transitionImageLayout(const vk::Image& image, const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout) const;

	void submit(const std::vector<vk::CommandBuffer>& commandBuffers, const vk::Semaphore& waitSemaphore, const vk::Semaphore& signalSemaphore, const vk::Fence& flightFence) const;

	void submit(const std::vector<DrawCommand>& drawCommands, const SyncObjects& syncObjects) const;

	[[nodiscard]] vk::Result present(const vk::SwapchainKHR& swapchain, std::uint32_t imageIndex, const vk::Semaphore& signalSemaphores) const;

	[[nodiscard]] vk::UniqueImageView createTextureImageViewUnique(const vk::Image& image) const;

	[[nodiscard]] vk::UniqueSampler createTextureSamplerUnique() const;

	std::shared_ptr<Allocator> createAllocator(const vk::UniqueInstance& instance) const;

private:
	vk::PhysicalDevice physicalDevice{};
	vk::UniqueDevice device{};
	vk::UniqueCommandPool commandPool{};
	vk::PhysicalDeviceProperties properties{};

	Queues queues{};
};
} // namespace vkx