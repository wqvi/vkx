#pragma once

#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>

namespace vkx {
class Mesh {
public:
	Mesh() = default;

	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const Allocator& allocator);

	explicit Mesh(const std::vector<vkx::Vertex>& vertices, const std::vector<std::uint32_t>& indices, VmaAllocator allocator);

	template <std::size_t T, std::size_t K>
	explicit Mesh(const std::array<Vertex, T>& vertices, const std::array<std::uint32_t, K>& indices, const Allocator& allocator)
	    : vertex(allocator.allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator.allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(K) {}

	template <class T, class K>
	explicit Mesh(const std::vector<T>& vertices, const std::vector<K>& indices, const Allocator& allocator)
	    : vertex(allocator.allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator.allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(indices.size()) {}

	template <class T, std::size_t K, class U, std::size_t Y>
	explicit Mesh(const std::array<T, K>& vertices, const std::array<U, Y>& indices, const Allocator& allocator)
	    : vertex(allocator.allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator.allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(indices.size()) {}

	std::shared_ptr<Allocation<vk::Buffer>> vertex;
	VkBuffer vertexBuffer = nullptr;
	VmaAllocation vertexAllocation = nullptr;
	VmaAllocationInfo vertexAllocationInfo{};
	std::shared_ptr<Allocation<vk::Buffer>> index;
	VkBuffer indexBuffer = nullptr;
	VmaAllocation indexAllocation = nullptr;
	VmaAllocationInfo indexAllocationInfo{};
	std::size_t indexCount = 0;
};

class Texture {
public:
	Texture() = default;

	explicit Texture(const char* file, VkDevice device, float maxAnisotropy, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] vk::DescriptorImageInfo createDescriptorImageInfo() const;

	[[nodiscard]] const vk::DescriptorImageInfo* getInfo() const;

	void destroy(VmaAllocator allocator, VkDevice device) const;

private:
	Image image;
	VkImageView view;
	VkSampler sampler;
	vk::DescriptorImageInfo info;
};
} // namespace vkx
