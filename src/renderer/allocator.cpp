#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/commands.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/allocator.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Buffer::Buffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo)
    : allocator(allocator), buffer(buffer), allocation(allocation), allocationSize(allocationInfo.size), mappedData(allocationInfo.pMappedData) {}

void vkx::Buffer::destroy() const {
	vmaDestroyBuffer(allocator, buffer, allocation);
}

void vkx::Buffer::mapMemory(const void* data) const {
	std::memcpy(mappedData, data, allocationSize);
}

vkx::Buffer::operator vk::Buffer() const {
	return static_cast<vk::Buffer>(buffer);
}

std::size_t vkx::Buffer::size() const {
	return allocationSize;
}
