#include <vkx/renderer/allocator.hpp>
#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/commands.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/renderer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vkx::VulkanAllocationDeleter::VulkanAllocationDeleter(VmaAllocator allocator)
    : allocator(allocator) {
}

void vkx::VulkanAllocationDeleter::operator()(VmaAllocation allocation) const noexcept {
	if (allocator) {
		vmaFreeMemory(allocator, allocation);
	}
}

vkx::VulkanPoolDeleter::VulkanPoolDeleter(VmaAllocator allocator)
    : allocator(allocator) {
}

void vkx::VulkanPoolDeleter::operator()(VmaPool pool) const noexcept {
	if (allocator) {
		vmaDestroyPool(allocator, pool);
	}
}

vkx::VulkanBufferMemoryPool::VulkanBufferMemoryPool(std::size_t blockSize,
						    std::size_t maxBlockCount,
						    vk::BufferUsageFlags bufferFlags,
						    VmaAllocator allocator,
						    vk::Device logicalDevice,
						    vkx::UniqueVulkanPool&& pool)
    : blockSize(blockSize),
      maxBlockCount(maxBlockCount),
      bufferFlags(bufferFlags),
      allocator(allocator),
      logicalDevice(logicalDevice),
      pool(std::move(pool)) {
}

std::vector<vkx::Buffer> vkx::VulkanBufferMemoryPool::allocateBuffers() const {
	std::vector<vkx::Buffer> buffers{};
	buffers.reserve(maxBlockCount);

	const vk::BufferCreateInfo bufferCreateInfo{{}, blockSize, bufferFlags, vk::SharingMode::eExclusive};

	const VmaAllocationCreateInfo allocationCreateInfo{
	    {},
	    {},
	    0,
	    0,
	    0,
	    pool.get(),
	    nullptr,
	    {}};

	for (auto i = 0; i < maxBlockCount; i++) {
		VkBuffer cBuffer = nullptr;
		VmaAllocation cAllocation = nullptr;
		VmaAllocationInfo cAllocationInfo;
		if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &cBuffer, &cAllocation, &cAllocationInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate GPU buffer.");
		}

		buffers.emplace_back(vk::UniqueBuffer(cBuffer, logicalDevice), UniqueVulkanAllocation(cAllocation, VulkanAllocationDeleter{allocator}), std::move(cAllocationInfo));
	}

	return buffers;
}

vkx::VulkanImageMemoryPool::VulkanImageMemoryPool(vkx::UniqueVulkanPool&& pool)
    : pool(std::move(pool)) {
}

void vkx::VulkanAllocatorDeleter::operator()(VmaAllocator allocator) const noexcept {
	vmaDestroyAllocator(allocator);
}

vkx::VulkanAllocator::VulkanAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice)
    : logicalDevice(logicalDevice) {
	constexpr VmaVulkanFunctions vulkanFunctions{
	    &vkGetInstanceProcAddr,
	    &vkGetDeviceProcAddr};

	const VmaAllocatorCreateInfo allocatorCreateInfo{
	    0,
	    static_cast<VkPhysicalDevice>(physicalDevice),
	    static_cast<VkDevice>(logicalDevice),
	    0,
	    nullptr,
	    nullptr,
	    nullptr,
	    &vulkanFunctions,
	    instance,
	    VK_API_VERSION_1_0,
#ifdef VMA_EXTERNAL_MEMORY
	    nullptr
#endif
	};

	auto cAllocator = vkx::create<VmaAllocator>(
	    vmaCreateAllocator,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create vulkan memory allocator.");
		    }
	    },
	    &allocatorCreateInfo);

	allocator = vkx::alloc::UniqueVmaAllocator{cAllocator, {&vmaDestroyAllocator}};
}

vkx::VulkanAllocator::operator VmaAllocator() const {
	return allocator.get();
}

vkx::Buffer vkx::VulkanAllocator::allocateBuffer(std::size_t memorySize,
						 vk::BufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags,
						 VmaMemoryUsage memoryUsage) const {
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

	return vkx::Buffer{vk::UniqueBuffer(cBuffer, logicalDevice), UniqueVulkanAllocation(cAllocation, VulkanAllocationDeleter{allocator.get()}), std::move(cAllocationInfo)};
}

vkx::Image vkx::VulkanAllocator::allocateImage(vk::Extent2D extent,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags,
					       VmaMemoryUsage memoryUsage) const {
	const vk::Extent3D imageExtent{extent.width, extent.height, 1};

	const vk::ImageCreateInfo imageCreateInfo{
	    {},
	    vk::ImageType::e2D,
	    format,
	    imageExtent,
	    1,
	    1,
	    vk::SampleCountFlagBits::e1,
	    tiling,
	    imageUsage,
	    vk::SharingMode::eExclusive};

	VmaAllocationCreateInfo allocationCreateInfo{
	    flags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	VkImage resourceImage = nullptr;
	VmaAllocation resourceAllocation = nullptr;
	if (vmaCreateImage(allocator.get(), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &resourceImage, &resourceAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	return vkx::Image{logicalDevice, vk::UniqueImage(resourceImage, logicalDevice), UniqueVulkanAllocation(resourceAllocation, VulkanAllocationDeleter{allocator.get()})};
}

vkx::Image vkx::VulkanAllocator::allocateImage(const vkx::CommandSubmitter& commandSubmitter,
					       const std::string& file,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags,
					       VmaMemoryUsage memoryUsage) const {
	int textureWidth = 0;
	int textureHeight = 0;
	int textureChannels = 0;
	auto* pixels = stbi_load(file.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image.");
	}

	const auto imageSize = static_cast<vk::DeviceSize>(textureWidth) * textureHeight * STBI_rgb_alpha;

	const auto stagingBuffer = allocateBuffer(pixels, imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	const vk::Extent3D imageExtent{static_cast<std::uint32_t>(textureWidth), static_cast<std::uint32_t>(textureHeight), 1};

	const vk::ImageCreateInfo imageCreateInfo{
	    {},
	    vk::ImageType::e2D,
	    format,
	    imageExtent,
	    1,
	    1,
	    vk::SampleCountFlagBits::e1,
	    tiling,
	    imageUsage,
	    vk::SharingMode::eExclusive};

	VmaAllocationCreateInfo allocationCreateInfo{
	    flags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	VkImage resourceImage = nullptr;
	VmaAllocation resourceAllocation = nullptr;
	if (vmaCreateImage(allocator.get(), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &resourceImage, &resourceAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	commandSubmitter.transitionImageLayout(resourceImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	commandSubmitter.copyBufferToImage(static_cast<vk::Buffer>(stagingBuffer), resourceImage, textureWidth, textureHeight);

	commandSubmitter.transitionImageLayout(resourceImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	stbi_image_free(pixels);
	return vkx::Image{logicalDevice, vk::UniqueImage(resourceImage, logicalDevice), UniqueVulkanAllocation(resourceAllocation, VulkanAllocationDeleter{allocator.get()})};
}

vkx::UniformBuffer vkx::VulkanAllocator::allocateUniformBuffer(std::size_t memorySize) const {
	return vkx::UniformBuffer{allocateBuffer(memorySize, vk::BufferUsageFlagBits::eUniformBuffer)};
}

std::vector<vkx::UniformBuffer> vkx::VulkanAllocator::allocateUniformBuffers(std::size_t memorySize, std::size_t amount) const {
	std::vector<vkx::UniformBuffer> uniformBuffers;
	uniformBuffers.reserve(amount);
	for (auto i = 0; i < amount; i++) {
		uniformBuffers.emplace_back(allocateBuffer(memorySize, vk::BufferUsageFlagBits::eUniformBuffer));
	}
	return uniformBuffers;
}

vkx::VulkanBufferMemoryPool vkx::VulkanAllocator::allocateBufferPool(vk::BufferUsageFlags bufferFlags,
								     std::size_t blockSize,
								     std::size_t maxBlockCount,
								     VmaAllocationCreateFlags flags,
								     VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo{{}, 0x10000, bufferFlags, vk::SharingMode::eExclusive};

	VmaAllocationCreateInfo allocationCreateInfo{
	    flags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	std::uint32_t memoryTypeIndex = 0;
	if (vmaFindMemoryTypeIndexForBufferInfo(allocator.get(), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &memoryTypeIndex) != VK_SUCCESS) {
		throw std::runtime_error("Failed to find memory type index for allocating a pool of buffers.");
	}

	VmaPoolCreateInfo poolCreateInfo{
	    memoryTypeIndex,
	    {},
	    blockSize,
	    {},
	    maxBlockCount,
	    {},
	    {},
	    nullptr};

	VmaPool pool = nullptr;
	if (vmaCreatePool(allocator.get(), &poolCreateInfo, &pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan memory pool.");
	}

	return vkx::VulkanBufferMemoryPool{blockSize, maxBlockCount, bufferFlags, allocator.get(), logicalDevice, vkx::UniqueVulkanPool(pool, VulkanPoolDeleter(allocator.get()))};
}

vkx::UniqueVulkanPool vkx::VulkanAllocator::allocatePool(vk::Extent2D extent,
							 vk::Format format,
							 vk::ImageTiling tiling,
							 vk::ImageUsageFlags imageUsage,
							 std::size_t blockSize,
							 std::size_t maxBlockCount,
							 VmaAllocationCreateFlags flags,
							 VmaMemoryUsage memoryUsage) const {
	const vk::Extent3D imageExtent{extent.width, extent.height, 1};

	const vk::ImageCreateInfo imageCreateInfo{
	    {},
	    vk::ImageType::e2D,
	    format,
	    imageExtent,
	    1,
	    1,
	    vk::SampleCountFlagBits::e1,
	    tiling,
	    imageUsage,
	    vk::SharingMode::eExclusive};

	VmaAllocationCreateInfo allocationCreateInfo{
	    flags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	std::uint32_t memoryTypeIndex = 0;
	if (vmaFindMemoryTypeIndexForImageInfo(allocator.get(), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &memoryTypeIndex) != VK_SUCCESS) {
		throw std::runtime_error("Failed to find memory type index for allocating a pool of buffers.");
	}

	VmaPoolCreateInfo poolCreateInfo{
	    memoryTypeIndex,
	    {},
	    blockSize,
	    {},
	    maxBlockCount,
	    {},
	    {},
	    nullptr};

	VmaPool pool = nullptr;
	if (vmaCreatePool(allocator.get(), &poolCreateInfo, &pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan memory pool.");
	}

	return vkx::UniqueVulkanPool(pool, VulkanPoolDeleter(allocator.get()));
}