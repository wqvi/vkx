#include <vkx/renderer/buffers.hpp>

vkx::UniformBuffer::UniformBuffer(vkx::Buffer&& buffer)
    : info(static_cast<vk::Buffer>(buffer), 0, buffer.size()),
      buffer(std::move(buffer)) {}

const vk::DescriptorBufferInfo* vkx::UniformBuffer::getInfo() const noexcept {
	return &info;
}
