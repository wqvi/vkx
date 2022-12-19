#include <vkx/renderer/model.hpp>

vkx::Buffer::Buffer(VmaAllocator allocator,
		    const void* data,
		    std::size_t memorySize,
		    VkBufferUsageFlags bufferFlags,
		    VmaAllocationCreateFlags allocationFlags,
		    VmaMemoryUsage memoryUsage)
    : allocator(allocator) {
	const VkBufferCreateInfo bufferCreateInfo{
	    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    nullptr,
	    0,
	    memorySize,
	    bufferFlags,
	    VK_SHARING_MODE_EXCLUSIVE,
	    0,
	    nullptr};

	const VmaAllocationCreateInfo allocationCreateInfo{
	    allocationFlags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate GPU buffer.");
	}

	if (data != nullptr) {
		std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
	}
}

vkx::Buffer::~Buffer() {
	if (buffer != nullptr) {
		vmaDestroyBuffer(allocator, buffer, allocation);
	}
}

void vkx::Buffer::mapMemory(const void* data) {
	std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
}

vkx::Mesh::Mesh(const void* vertexData, std::size_t vertexSize, const void* indexData, std::size_t indexSize, VmaAllocator allocator) {
	vertexAllocation = vkx::allocateBuffer(&vertexAllocationInfo, &vertexBuffer, allocator, vertexData, vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	indexAllocation = vkx::allocateBuffer(&indexAllocationInfo, &indexBuffer, allocator, indexData, indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

vkx::Mesh::Mesh(const std::vector<vkx::Vertex>& vertices, const std::vector<std::uint32_t>& indices, VmaAllocator allocator) {
	const auto* vertexData = vertices.data();
	const auto vertexSize = vertices.size() * sizeof(vkx::Vertex);
	const auto* indexData = indices.data();
	const auto indexSize = indices.size() * sizeof(std::uint32_t);

	vertexAllocation = vkx::allocateBuffer(&vertexAllocationInfo, &vertexBuffer, allocator, vertexData, vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	indexAllocation = vkx::allocateBuffer(&indexAllocationInfo, &indexBuffer, allocator, indexData, indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void vkx::Mesh::destroy(VmaAllocator allocator) const {
	vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
	vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
}

vkx::Texture::Texture(const char* file, VkDevice device, float maxAnisotropy, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter)
    : image(file, allocator, commandSubmitter),
      view(image.createTextureImageView(device)),
      sampler(vkx::createTextureSampler(device, maxAnisotropy)),
      info{sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      device(device) {}

VkDescriptorImageInfo vkx::Texture::createDescriptorImageInfo() const {
	return {sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
}

const VkDescriptorImageInfo* vkx::Texture::getInfo() const {
	return &info;
}

void vkx::Texture::destroy() const {
	image.destroy();
	vkDestroyImageView(device, view, nullptr);
	vkDestroySampler(device, sampler, nullptr);
}