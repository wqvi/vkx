#pragma once

namespace vkx {
class Image;
class CommandSubmitter;
struct UniformBuffer;

class Buffer {
private:
	VmaAllocator allocator;
	VkBuffer buffer;
	VmaAllocation allocation;
	std::size_t allocationSize;
	void* mappedData;

public:
	Buffer() = default;

	explicit Buffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo);

	explicit operator vk::Buffer() const;

	void destroy() const;

	void mapMemory(const void* data) const;

	std::size_t size() const;
};
} // namespace vkx
