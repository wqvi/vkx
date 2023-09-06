#pragma once

namespace vkx {
class Image;
class CommandSubmitter;
struct UniformBuffer;

class Buffer {
private:
	vk::UniqueBuffer buffer;
	VmaAllocation allocation{};
	VmaAllocationInfo allocationInfo{};

public:
	Buffer() = default;

	explicit Buffer(vk::UniqueBuffer&& buffer, VmaAllocation allocation, VmaAllocationInfo&& allocationInfo);

	explicit operator vk::Buffer() const;

	template <class T>
	void mapMemory(const T* data) const {
		std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
	}

	template <class T>
	void mapMemory(const T* data, std::size_t memoryOffset) const {
		T* ptr = reinterpret_cast<T*>(allocationInfo.pMappedData) + memoryOffset;
		const auto size = allocationInfo.size - memoryOffset;
		std::memcpy(ptr, data, size);
	}

	std::size_t size() const;
};

class VulkanBufferMemoryPool {
private:
	std::size_t blockSize = 0;
	std::size_t maxBlockCount = 0;
	vk::BufferUsageFlags bufferFlags{};
	VmaAllocator allocator{};
	vk::Device logicalDevice{};
	VmaPool pool{};

public:
	VulkanBufferMemoryPool() = default;

	explicit VulkanBufferMemoryPool(std::size_t blockSize,
					std::size_t maxBlockCount,
					vk::BufferUsageFlags bufferFlags,
					VmaAllocator allocator,
					vk::Device logicalDevice,
					VmaPool pool);

	[[nodiscard]] std::vector<vkx::Buffer> allocateBuffers() const;
};

class VulkanAllocator {
private:
	vk::Device logicalDevice{};
	VmaAllocator allocator{};

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
		if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &cBuffer, &cAllocation, &cAllocationInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate GPU buffer.");
		}

		vkx::Buffer buffer{vk::UniqueBuffer{cBuffer, logicalDevice},
				   cAllocation,
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
};
} // namespace vkx
