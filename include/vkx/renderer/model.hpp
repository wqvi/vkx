#pragma once

#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/vertex.hpp>

namespace vkx {
struct Mesh {
	vkx::Buffer vertexBuffer{};
	vkx::Buffer indexBuffer{};
	std::vector<vkx::Vertex> vertices{};
	std::vector<std::uint32_t> indices{};
	std::size_t activeIndexCount = 0;

	Mesh() = default;

	explicit Mesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanAllocator& allocator);

	explicit Mesh(std::size_t vertexCount, std::size_t indexCount, const vkx::VulkanAllocator& allocator);
};

struct ArrayMesh {
	vkx::UniqueVulkanPool vertexPool{};
	vkx::UniqueVulkanPool indexPool{};

	ArrayMesh() = default;

	explicit ArrayMesh(const vkx::VulkanAllocator& allocator);
};
} // namespace vkx
