#pragma once

#include <vkx/renderer/core/renderer_types.hpp>
#include <vk_mem_alloc.h>

namespace vkx {
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

	template <class T>
	std::vector<vkx::UniformBuffer<T>> allocateUniformBuffers(const T& value = {}) const;

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
		std::memcpy(allocationInfo.pMappedData, memory.data(), allocationInfo.size);
	}

	template <class K, std::size_t size>
	void mapMemory(const std::array<K, size>& memory) const {
		std::memcpy(allocationInfo.pMappedData, memory.data(), allocationInfo.size);
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

template <class T>
std::vector<vkx::UniformBuffer<T>> Allocator::allocateUniformBuffers(const T& value) const {
	std::vector<vkx::UniformBuffer<T>> buffers;
	buffers.reserve(MAX_FRAMES_IN_FLIGHT);
	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		buffers.emplace_back(value, allocateBuffer(value, vk::BufferUsageFlagBits::eUniformBuffer));
	}
	return buffers;
}
}