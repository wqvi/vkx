#pragma once

#include "vkx/renderer/core/allocable.hpp"
#include "vkx/renderer/core/device.hpp"
#include "vk_mem_alloc.h"

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

    // VMA testing section
//    VkBuffer InBufferRaw;
//    VkBuffer OutBufferRaw;
//
//    VmaAllocationCreateInfo AllocationInfo = {};
//    AllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//
//    vk::BufferCreateInfo bufferInfo{
//            {},
//            size,
//            usage,
//            vk::SharingMode::eExclusive
//    };
//
//    auto *foobar = device.allocator.get();
//
//    auto barfoo = static_cast<VmaAllocator>(*foobar);
//
//    VmaAllocation InBufferAllocation;
//    vmaCreateBuffer(barfoo,
//                    reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
//                    &AllocationInfo,
//                    &InBufferRaw,
//                    &InBufferAllocation,
//                    nullptr);
//
//    AllocationInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
//    VmaAllocation OutBufferAllocation;
//    vmaCreateBuffer(barfoo,
//                    reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
//                    &AllocationInfo,
//                    &OutBufferRaw,
//                    &OutBufferAllocation,
//                    nullptr);
//
//    vk::Buffer InBuffer = InBufferRaw;
//    vk::Buffer OutBuffer = OutBufferRaw;
//
//    int32_t* InBufferPtr = nullptr;
//    vmaMapMemory(barfoo, InBufferAllocation, reinterpret_cast<void**>(&InBufferPtr));
//    for (int32_t I = 0; I < 1; ++I)
//    {
//        InBufferPtr[I] = I;
//    }
//    vmaUnmapMemory(barfoo, InBufferAllocation);
}