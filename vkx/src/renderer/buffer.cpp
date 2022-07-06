#include <renderer/buffer.hpp>

#include <renderer/core/device.hpp>

namespace vkx {
    Buffer::Buffer(std::vector<Vertex> const &vertices, Device const &device)
            : Buffer(vertices.data(), vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer,
                     device) {}

    Buffer::Buffer(std::vector<std::uint32_t> const &indices, Device const &device)
            : Buffer(indices.data(), indices.size() * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer,
                     device) {}

    Buffer::Buffer(std::size_t size, vk::BufferUsageFlags const &usage, Device const &device) {
        obj = device.createBufferUnique(size, vk::BufferUsageFlagBits::eTransferDst | usage);
        memory = device.allocateMemoryUnique(obj, vk::MemoryPropertyFlagBits::eDeviceLocal);
    }

    template<class T>
    Buffer::Buffer(T const *data, std::size_t size, vk::BufferUsageFlags const &usage, Device const &device) {
        auto stagingBuffer = device.createBufferUnique(size, vk::BufferUsageFlagBits::eTransferSrc);
        auto stagingBufferMemory = device.allocateMemoryUnique(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible |
                                                                              vk::MemoryPropertyFlagBits::eHostCoherent);

        auto *mappedMemory = static_cast<T *>(device->mapMemory(*stagingBufferMemory, 0, size, {}));
        std::memcpy(mappedMemory, data, size);
        device->unmapMemory(*stagingBufferMemory);

        obj = device.createBufferUnique(size, vk::BufferUsageFlagBits::eTransferDst | usage);
        memory = device.allocateMemoryUnique(obj, vk::MemoryPropertyFlagBits::eDeviceLocal);

        device.copyBuffer(*stagingBuffer, *obj, size);
    }
}

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