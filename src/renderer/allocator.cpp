#include <vkx/renderer/allocator.hpp>

vkx::VulkanAllocationDeleter::VulkanAllocationDeleter(VmaAllocator allocator)
    : allocator(allocator) {
}

void vkx::VulkanAllocationDeleter::operator()(VmaAllocation allocation) const noexcept {
	if (allocator) {
		vmaFreeMemory(allocator, allocation);
	}
}

vkx::VulkanPoolDeleter::VulkanPoolDeleter(VmaAllocator allocator)
    : allocator(allocator) {
}

void vkx::VulkanPoolDeleter::operator()(VmaPool pool) const noexcept {
	if (allocator) {
		vmaDestroyPool(allocator, pool);
	}
}

void vkx::VulkanAllocatorDeleter::operator()(VmaAllocator allocator) const noexcept {
	vmaDestroyAllocator(allocator);
}