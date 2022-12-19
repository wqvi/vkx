#pragma once

#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Mesh {
public:
	Mesh() = default;

	explicit Mesh(const void* vertexData, std::size_t vertexSize, const void* indexData, std::size_t indexSize, VmaAllocator allocator);

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
