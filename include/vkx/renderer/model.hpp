#pragma once

#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>

namespace vkx {
class Mesh {
public:
	Mesh() = default;

	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const std::shared_ptr<Allocator>& allocator);

	template <std::size_t T, std::size_t K>
	explicit Mesh(const std::array<Vertex, T>& vertices, const std::array<std::uint32_t, K>& indices, const std::shared_ptr<Allocator>& allocator)
	    : vertex(allocator->allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator->allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(K) {}

	template <class T, class K>
	explicit Mesh(const std::vector<T>& vertices, const std::vector<K>& indices, const std::shared_ptr<Allocator>& allocator)
	    : vertex(allocator->allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator->allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(indices.size()) {}

	std::shared_ptr<Allocation<vk::Buffer>> vertex;
	std::shared_ptr<Allocation<vk::Buffer>> index;
	std::size_t indexCount = 0;
};

class Texture : public ShaderDescriptor<vk::DescriptorImageInfo> {
public:
	Texture() = default;

	Texture(const std::string& file, const Device& device, const std::shared_ptr<vkx::Allocator>& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter);

	[[nodiscard]] vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet descriptorSet, std::uint32_t dstBinding) const;

	[[nodiscard]] vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorImageInfo* imageInfo, vk::DescriptorSet descriptorSet, std::uint32_t dstBinding) const;

	[[nodiscard]] vk::DescriptorImageInfo createDescriptorImageInfo() const;

	vk::DescriptorImageInfo getInfo() const override;

private:
	Image image;
	vk::UniqueImageView view;
	vk::UniqueSampler sampler;
};
} // namespace vkx
