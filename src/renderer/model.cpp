#include "vkx/renderer/core/renderer_types.hpp"
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const vkx::Allocator& allocator)
    : vertex(allocator.allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator.allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(indices.size()) {}

vkx::Texture::Texture(const std::string& file, vk::Device device, float maxAnisotropy, const vkx::Allocator& allocator, const vkx::CommandSubmitter& commandSubmitter)
    : image(file, allocator, commandSubmitter),
      view(vkx::createTextureImageView(device, image.resource->object), device),
      sampler(vkx::createTextureSamplerUnique(device, maxAnisotropy)),
      info(*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal) {
}

vk::DescriptorImageInfo vkx::Texture::createDescriptorImageInfo() const {
	return {*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal};
}

const vk::DescriptorImageInfo* vkx::Texture::getInfo() const {
	return &info;
}
