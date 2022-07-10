#pragma once

#include <renderer/core/allocable.hpp>
#include <renderer/core/device.hpp>

namespace vkx {
    class BufferBase {
    public:
        explicit operator vk::Buffer const &() const noexcept;

        explicit operator vk::UniqueBuffer const &() const noexcept;

        [[nodiscard("You most likely are causing an error if you discard this buffer memory")]]
        vk::UniqueDeviceMemory const &getMemory() const noexcept;

    protected:
        BufferBase() = default;

        template<class T>
        explicit BufferBase(T const *data,
                            std::size_t size,
                            Device const &device);

    private:
        vk::UniqueBuffer buffer;
        vk::UniqueDeviceMemory memory;
    };

    class VertexBuffer : public BufferBase {
    public:
        VertexBuffer() = default;

        explicit VertexBuffer(std::vector<Vertex> const &vertices, Device const &device);
    };

    class IndexBuffer : public BufferBase {
    public:
        IndexBuffer() = default;

        explicit IndexBuffer(std::vector<std::uint32_t> const &indices, Device const &device);
    };
}

template<class T>
vkx::BufferBase::BufferBase(T const *data, std::size_t size, vkx::Device const &device) {
    // Create cpu side staging buffers
    auto stagingBuffer = device.createBufferUnique(
            size,
            vk::BufferUsageFlagBits::eTransferSrc);

    auto stagingBufferMemory = device.allocateMemoryUnique(
            stagingBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    // Populate cpu side staging buffers
    auto *mappedMemory = static_cast<T *>(device->mapMemory(
            *stagingBufferMemory,
            0,
            size,
            {}));
    std::memcpy(mappedMemory, data, size);
    device->unmapMemory(*stagingBufferMemory);

    // Make gpu sided buffer usage flags based on type of template parameter
    vk::BufferUsageFlagBits usage = vk::BufferUsageFlagBits::eVertexBuffer;
    if (std::is_same<T, std::uint32_t>::value) {
        usage = vk::BufferUsageFlagBits::eIndexBuffer;
    }

    // Allocate gpu size buffer
    buffer = device.createBufferUnique(
            size,
            vk::BufferUsageFlagBits::eTransferDst | usage);
    memory = device.allocateMemoryUnique(
            buffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

    // Copy cpu side buffer to gpu side buffer
    device.copyBuffer(*stagingBuffer, *buffer, size);
}