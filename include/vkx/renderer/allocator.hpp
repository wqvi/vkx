#pragma once

#include <vkx/renderer/memory/unique.hpp>
#include <vkx/renderer/types.hpp>

namespace vkx {
class VulkanBufferMemoryPool {
private:
	std::size_t blockSize = 0;
	std::size_t maxBlockCount = 0;
	vk::BufferUsageFlags bufferFlags{};
	VmaAllocator allocator = nullptr;
	vk::Device logicalDevice{};
	vkx::alloc::UniqueVmaPool pool{};

public:
	VulkanBufferMemoryPool() = default;

	explicit VulkanBufferMemoryPool(std::size_t blockSize,
					std::size_t maxBlockCount,
					vk::BufferUsageFlags bufferFlags,
					VmaAllocator allocator,
					vk::Device logicalDevice,
					vkx::alloc::UniqueVmaPool&& pool);

	[[nodiscard]] std::vector<vkx::Buffer> allocateBuffers() const;
};

struct VulkanAllocatorDeleter {
	void operator()(VmaAllocator allocator) const noexcept;
};

class VulkanAllocator {
private:
	vk::Device logicalDevice = nullptr;
	vkx::alloc::UniqueVmaAllocator allocator;

public:
	VulkanAllocator() = default;

	explicit VulkanAllocator(vk::Instance instance,
				 vk::PhysicalDevice physicalDevice,
				 vk::Device logicalDevice);

	explicit operator VmaAllocator() const;

	template <class T>
	[[nodiscard]] vkx::Buffer allocateBuffer(const T* data,
						 std::size_t memorySize,
						 vk::BufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
						 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const {
		const vk::BufferCreateInfo bufferCreateInfo{{}, memorySize, bufferFlags, vk::SharingMode::eExclusive};

		const VmaAllocationCreateInfo allocationCreateInfo{
		    allocationFlags,
		    memoryUsage,
		    0,
		    0,
		    0,
		    nullptr,
		    nullptr,
		    {}};

		VkBuffer cBuffer = nullptr;
		VmaAllocation cAllocation = nullptr;
		VmaAllocationInfo cAllocationInfo;
		if (vmaCreateBuffer(allocator.get(), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &cBuffer, &cAllocation, &cAllocationInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate GPU buffer.");
		}

		vkx::Buffer buffer{vk::UniqueBuffer{cBuffer, logicalDevice},
				   vkx::alloc::UniqueVmaAllocation{cAllocation, vkx::alloc::VmaAllocationDeleter{&vmaFreeMemory, allocator.get()}},
				   std::move(cAllocationInfo)};
		buffer.mapMemory(data);
		return buffer;
	}

	[[nodiscard]] vkx::Buffer allocateBuffer(std::size_t memorySize,
						 vk::BufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
						 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] vkx::Image allocateImage(vk::Extent2D extent,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
					       VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] vkx::Image allocateImage(const vkx::CommandSubmitter& commandSubmitter,
					       const std::string& file,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
					       VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] vkx::UniformBuffer allocateUniformBuffer(std::size_t memorySize) const;

	[[nodiscard]] std::vector<vkx::UniformBuffer> allocateUniformBuffers(std::size_t memorySize, std::size_t amount) const;

	[[nodiscard]] vkx::VulkanBufferMemoryPool allocateBufferPool(vk::BufferUsageFlags bufferFlags,
								     std::size_t blockSize,
								     std::size_t maxBlockCount,
								     VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
								     VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;

	[[nodiscard]] vkx::alloc::UniqueVmaPool allocatePool(vk::Extent2D extent,
							 vk::Format format,
							 vk::ImageTiling tiling, vk::ImageUsageFlags imageUsage,
							 std::size_t blockSize,
							 std::size_t maxBlockCount,
							 VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
							 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const;
};
} // namespace vkx