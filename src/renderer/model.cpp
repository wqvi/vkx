#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/model.hpp>

vkx::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const std::shared_ptr<Allocator>& allocator)
    : vertex(allocator->allocateBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer)), index(allocator->allocateBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer)), indexCount(indices.size()) {}

vkx::Texture::Texture(const std::string& file, const Device& device, const std::shared_ptr<vkx::Allocator>& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter)
    : image(file, allocator, commandSubmitter),
      view(device.createTextureImageViewUnique(image.resource->object)),
      sampler(device.createTextureSamplerUnique()) {
}

vk::WriteDescriptorSet vkx::Texture::createWriteDescriptorSet(vk::DescriptorSet descriptorSet, std::uint32_t dstBinding) const {
	static vk::DescriptorImageInfo info{};

	info = vk::DescriptorImageInfo{
	    *sampler,
	    *view,
	    vk::ImageLayout::eShaderReadOnlyOptimal};

	static vk::WriteDescriptorSet set{};

	set = vk::WriteDescriptorSet{
	    descriptorSet,
	    dstBinding,
	    0,
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    &info,
	    nullptr};

	return set;
}

vk::WriteDescriptorSet vkx::Texture::createWriteDescriptorSet(vk::DescriptorImageInfo* imageInfo, vk::DescriptorSet descriptorSet, std::uint32_t dstBinding) const {
	return {descriptorSet,
		dstBinding,
		0,
		1,
		vk::DescriptorType::eCombinedImageSampler,
		imageInfo,
		nullptr};
}

vk::DescriptorImageInfo vkx::Texture::createDescriptorImageInfo() const {
	return {*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal};
}

vk::DescriptorImageInfo vkx::Texture::getInfo() const {
	return {*sampler, *view, vk::ImageLayout::eShaderReadOnlyOptimal};
}