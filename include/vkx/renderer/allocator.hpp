#pragma once

namespace vkx {
class VulkanAllocationDeleter {
private:
	VmaAllocator allocator = nullptr;

public:
	VulkanAllocationDeleter() = default;

	explicit VulkanAllocationDeleter(VmaAllocator allocator);

	void operator()(VmaAllocation allocation) const noexcept;
};

using UniqueVulkanAllocation = std::unique_ptr<std::remove_pointer_t<VmaAllocation>, VulkanAllocationDeleter>;

class VulkanPoolDeleter {
private:
	VmaAllocator allocator = nullptr;

public:
	VulkanPoolDeleter() = default;

	explicit VulkanPoolDeleter(VmaAllocator allocator);

	void operator()(VmaPool pool) const noexcept;
};

using UniqueVulkanPool = std::unique_ptr<std::remove_pointer_t<VmaPool>, VulkanPoolDeleter>;

struct VulkanAllocatorDeleter {
	void operator()(VmaAllocator allocator) const noexcept;
};

using UniqueVulkanAllocator = std::unique_ptr<std::remove_pointer_t<VmaAllocator>, VulkanAllocatorDeleter>;

} // namespace vkx