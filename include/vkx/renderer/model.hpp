#pragma once

#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Mesh {
public:
	Mesh() = default;

	template <class T, std::size_t K, class U, std::size_t Y>
	explicit Mesh(const std::array<T, K>& vertices, const std::array<U, Y>& indices, VmaAllocator allocator)
	    : indexCount(indices.size()) {
		vertexAllocation = vkx::allocateBuffer(&vertexAllocationInfo, &vertexBuffer, allocator, vertices.data(), sizeof(T) * K, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		indexAllocation = vkx::allocateBuffer(&indexAllocationInfo, &indexBuffer, allocator, indices.data(), sizeof(U) * Y, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}

	void destroy(VmaAllocator allocator) const;
	
	VkBuffer vertexBuffer = nullptr;
	VmaAllocation vertexAllocation = nullptr;
	VmaAllocationInfo vertexAllocationInfo{};
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
