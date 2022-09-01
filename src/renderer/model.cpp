#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/model.hpp>

vkx::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const std::shared_ptr<Allocator>& allocator)
    : vertex(allocator->allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator->allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(indices.size()) {}

vkx::Texture::Texture(const std::string& file, const Device& device, const std::shared_ptr<vkx::Allocator>& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter)
    : image(file, allocator, commandSubmitter),
      view(device.createTextureImageViewUnique(image.resource->object)),
      sampler(device.createTextureSamplerUnique()) {
}

vk::DescriptorImageInfo vkx::Texture::createDescriptorImageInfo() const {
	return {*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal};
}