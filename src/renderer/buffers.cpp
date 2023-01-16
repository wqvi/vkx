#include <vkx/renderer/buffers.hpp>

vkx::Buffer::Buffer(vk::UniqueBuffer&& buffer, vkx::alloc::UniqueVmaAllocation&& allocation, VmaAllocationInfo&& allocationInfo)
    : buffer(std::move(buffer)), allocation(std::move(allocation)), allocationInfo(std::move(allocationInfo)) {}

vkx::Buffer::operator vk::Buffer() const {
	return *buffer;
}

std::size_t vkx::Buffer::size() const {
	return allocationInfo.size;
}

vkx::UniformBuffer::UniformBuffer(vkx::Buffer&& buffer)
    : info(static_cast<vk::Buffer>(buffer), 0, buffer.size()),
      buffer(std::move(buffer)) {}

const vk::DescriptorBufferInfo* vkx::UniformBuffer::getInfo() const noexcept {
	return &info;
}