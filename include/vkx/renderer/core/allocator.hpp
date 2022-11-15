#pragma once

#include <vkx/renderer/core/renderer_types.hpp>

namespace vkx {
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

template <class T>
template <class K>
void vkx::Allocation<T>::mapMemory(const std::vector<K>& memory) const {
	std::memcpy(allocationInfo.pMappedData, memory.data(), allocationInfo.size);
}

template <class T>
template <class K, std::size_t size>
void vkx::Allocation<T>::mapMemory(const std::array<K, size>& memory) const {
	std::memcpy(allocationInfo.pMappedData, memory.data(), allocationInfo.size);
}

template <>
inline vkx::Allocation<VkImage>::~Allocation() {
	vmaDestroyImage(allocator, object, allocation);
}

template <>
inline vkx::Allocation<VkBuffer>::~Allocation() {
	vmaDestroyBuffer(allocator, object, allocation);
}