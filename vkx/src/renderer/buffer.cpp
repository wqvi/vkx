#include <vkx/renderer/buffer.hpp>

#include <vkx/renderer/core/device.hpp>

vkx::BufferBase::operator vk::Buffer const &() const noexcept {
    return buffer.get();
}

vkx::BufferBase::operator vk::UniqueBuffer const &() const noexcept {
    return buffer;
}

vk::UniqueDeviceMemory const &vkx::BufferBase::getMemory() const noexcept {
    return memory;
}

vkx::VertexBuffer::VertexBuffer(std::vector<Vertex> const &vertices, Device const &device)
        : BufferBase(vertices.data(), vertices.size() * sizeof(Vertex), device) {}

vkx::IndexBuffer::IndexBuffer(std::vector<std::uint32_t> const &indices, Device const &device)
        : BufferBase(indices.data(), indices.size() * sizeof(Vertex), device) {}