#include "vkx/renderer/core/device.hpp"
#include <vkx/renderer/model.hpp>

namespace vkx {
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, const Device& device)
    : vertexBuffer(vertices, device), indexBuffer(indices, device), indexCount(indices.size()) {}

Texture::Texture(const std::string& file, const Device& device, const std::shared_ptr<Allocator>& allocator)
    : image(file, device, allocator),
      view(device.createTextureImageViewUnique(image.resource->object)),
      sampler(device.createTextureSamplerUnique()) {
}

vk::WriteDescriptorSet
Texture::createWriteDescriptorSet(const vk::DescriptorSet& descriptorSet, std::uint32_t dstBinding) const {
	static vk::DescriptorImageInfo info{};

	info = vk::DescriptorImageInfo{
	    *sampler,				    // sampler
	    *view,				    // imageView
	    vk::ImageLayout::eShaderReadOnlyOptimal // imageLayout
	};

	static vk::WriteDescriptorSet set{};

	set = vk::WriteDescriptorSet{
	    descriptorSet,			       // dstSet
	    dstBinding,				       // dstBinding
	    0,					       // dstArrayElement
	    1,					       // descriptorCount
	    vk::DescriptorType::eCombinedImageSampler, // descriptorType
	    &info,				       // pImageInfo
	    nullptr				       // pBufferInfo
	};
	return set;
}
} // namespace vkx

vkx::Model::Model(Mesh&& mesh, Texture&& texture, const Material& material)
    : mesh(std::move(mesh)), texture(std::move(texture)), material(material) {}

glm::mat4 vkx::Model::getModelMatrix() const noexcept {
	return glm::translate(glm::mat4(1), position);
}
