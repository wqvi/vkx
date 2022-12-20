#pragma once

#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Buffer {
private:
	VmaAllocator allocator = nullptr;
	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo{};

public:
	Buffer() = default;

	explicit Buffer(VmaAllocator allocator,
			const void* data,
			std::size_t memorySize,
			VkBufferUsageFlags bufferFlags,
			VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

	Buffer(const Buffer& buffer) = delete;

	Buffer(Buffer&& buffer) = default;

	~Buffer();

	Buffer& operator=(const Buffer& buffer) = delete;

	Buffer& operator=(Buffer&& buffer) = default;

	void mapMemory(const void* data);
};

class Mesh {
public:
	Mesh() = default;

	explicit Mesh(const void* vertexData, std::size_t vertexSize, const void* indexData, std::size_t indexSize, VmaAllocator allocator);

	explicit Mesh(const std::vector<vkx::Vertex>& vertices, const std::vector<std::uint32_t>& indices, VmaAllocator allocator);

	void destroy(VmaAllocator allocator) const;

	VkBuffer vertexBuffer = nullptr;
	VmaAllocation vertexAllocation = nullptr;
	VmaAllocationInfo vertexAllocationInfo{};
	VkBuffer indexBuffer = nullptr;
	VmaAllocation indexAllocation = nullptr;
	VmaAllocationInfo indexAllocationInfo{};
	std::size_t indexCount = 0;
};

class TestMesh {
private:
	vkx::Buffer vertexBuffer{};
	vkx::Buffer indexBuffer{};
	std::vector<vkx::Vertex> vertices{};
	std::vector<std::uint32_t> indices{};
	std::size_t activeIndexCount = 0;

public:
	TestMesh() = default;

	explicit TestMesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, VmaAllocator allocator);
};

class Texture {
public:
	explicit Texture(const char* file, VkDevice device, float maxAnisotropy, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] VkDescriptorImageInfo createDescriptorImageInfo() const;

	[[nodiscard]] const VkDescriptorImageInfo* getInfo() const;

	void destroy() const;

private:
	Image image;
	VkImageView view = nullptr;
	VkSampler sampler = nullptr;
	VkDescriptorImageInfo info{};
	VkDevice device = nullptr;
};
} // namespace vkx
