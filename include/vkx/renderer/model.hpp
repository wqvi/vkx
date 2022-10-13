#pragma once

#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>

namespace vkx {
class Mesh {
public:
	Mesh() = default;

	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const Allocator& allocator);

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
	std::shared_ptr<Allocation<vk::Buffer>> index;
	std::size_t indexCount = 0;
};

class Texture {
public:
	Texture() = default;

	Texture(const std::string& file, const Device& device, const Allocator& allocator, const vkx::CommandSubmitter& commandSubmitter);

	Texture(const std::string& file, vk::Device device, const vkx::Allocator& allocator, const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] vk::DescriptorImageInfo createDescriptorImageInfo() const;

	[[nodiscard]] const vk::DescriptorImageInfo* getInfo() const;

private:
	Image image;
	vk::UniqueImageView view;
	vk::UniqueSampler sampler;
	vk::DescriptorImageInfo info;
};
} // namespace vkx
