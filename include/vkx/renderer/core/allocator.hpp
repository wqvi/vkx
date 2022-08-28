#pragma once

#include <vkx/renderer/core/renderer_types.hpp>

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
	void mapMemory(const std::vector<K>& memory) const;

	template <class K, std::size_t size>
	void mapMemory(const std::array<K, size>& memory) const;
};
} // namespace vkx

#include "allocator.inl"