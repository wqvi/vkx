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

vkx::Buffer::Buffer(Buffer&& other) noexcept
    : allocator(std::move(other.allocator)),
      buffer(std::move(other.buffer)),
      allocation(std::move(other.allocation)),
      allocationInfo(std::move(other.allocationInfo)) {
	other.allocator = nullptr;
	other.buffer = nullptr;
	other.allocation = nullptr;
	std::memset(&other.allocationInfo, 0, sizeof(VmaAllocationInfo));
}

vkx::Buffer::~Buffer() {
	if (buffer) {
		vmaDestroyBuffer(allocator, buffer, allocation);
	}
}

vkx::Buffer& vkx::Buffer::operator=(Buffer&& other) noexcept {
	allocator = std::move(other.allocator);
	buffer = std::move(other.buffer);
	allocation = std::move(other.allocation);
	allocationInfo = std::move(other.allocationInfo);

	other.allocator = nullptr;
	other.buffer = nullptr;
	other.allocation = nullptr;
	std::memset(&other.allocationInfo, 0, sizeof(VmaAllocationInfo));
	return *this;
}

vkx::Buffer::operator VkBuffer() const {
	return buffer;
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

vkx::TestMesh::TestMesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, VmaAllocator allocator)
    : vertexBuffer(allocator, vertices.data(), vertices.size() * sizeof(vkx::Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
      indexBuffer(allocator, indices.data(), indices.size() * sizeof(std::uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
      vertices(std::move(vertices)),
      indices(std::move(indices)),
      activeIndexCount(activeIndexCount) {
}

const vkx::Buffer& vkx::TestMesh::getVertexBuffer() const {
	return vertexBuffer;
}

const vkx::Buffer& vkx::TestMesh::getIndexBuffer() const {
	return indexBuffer;
}

std::size_t vkx::TestMesh::getActiveIndexCount() const {
	return activeIndexCount;
}
