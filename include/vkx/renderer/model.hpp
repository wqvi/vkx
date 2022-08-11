#pragma once

#include "vkx/renderer/core/device.hpp"
#include "vkx/renderer/core/vertex.hpp"
#include <array>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

namespace vkx {
class Mesh {
public:
	Mesh() = default;

	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const Device& device, const std::shared_ptr<Allocator>& allocator);

	template <std::size_t T, std::size_t K>
	explicit Mesh(const std::array<Vertex, T>& vertices, const std::array<std::uint32_t, K>& indices, const std::shared_ptr<Allocator>& allocator)
		: vertex(allocator->allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator->allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(K) {}

	std::shared_ptr<Allocation<vk::Buffer>> vertex;
	std::shared_ptr<Allocation<vk::Buffer>> index;
	std::size_t indexCount = 0;
};

class Texture {
public:
	Texture() = default;

	Texture(const std::string& file, const Device& device, const std::shared_ptr<vkx::Allocator>& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter);

	[[nodiscard]] vk::WriteDescriptorSet createWriteDescriptorSet(const vk::DescriptorSet& descriptorSet, std::uint32_t dstBinding) const;

private:
	Image image;
	vk::UniqueImageView view;
	vk::UniqueSampler sampler;
};

struct Model {
	Model() = default;

	explicit Model(Mesh&& mesh, Texture&& texture, const Material& material);

	[[nodiscard]] glm::mat4 getModelMatrix() const noexcept;

	Mesh mesh;
	Texture texture;
	Material material = {};
	glm::vec3 position = glm::vec3(0);
};
} // namespace vkx
