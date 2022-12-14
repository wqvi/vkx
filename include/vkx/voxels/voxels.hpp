#pragma once

#include <vkx/renderer/core/vertex.hpp>
#include <vkx/renderer/model.hpp>

namespace vkx {
template <class T>
constexpr std::size_t flattenIndex(std::size_t size, std::size_t accumulator, T index) {
	return accumulator * index;
}

template <class T, class... Y>
constexpr std::size_t flattenIndex(std::size_t size, std::size_t accumulator, T x, Y... indices) {
	const auto index = flattenIndex(size, accumulator * size, indices...);
	return static_cast<std::size_t>(x) * accumulator + index;
}

template <class T, class... Y>
constexpr std::size_t flattenIndex(std::size_t size, T x, Y... indices) {
	const auto index = flattenIndex(size, size, indices...);
	return static_cast<std::size_t>(x) + index;
}

enum class Voxel : std::uint32_t {
	Air,
	Stone
};

struct VoxelMask {
	Voxel voxel = Voxel::Air;
	std::int32_t normal = 0;

	bool operator==(const VoxelMask& other) const;

	bool operator!=(const VoxelMask& other) const;
};

class VoxelChunk {
private:
	using Mask = std::vector<vkx::VoxelMask>;

public:
	std::int32_t size;

	glm::ivec3 chunkPosition;
	std::vector<Voxel> voxels;
	std::vector<vkx::Vertex> vertices;
	std::vector<std::uint32_t> indices;

	std::uint32_t vertexCount;

	std::vector<vkx::Vertex>::iterator vertexIter;
	std::vector<std::uint32_t>::iterator indexIter;

	glm::ivec3 normalizedPosition;

	std::vector<vkx::VoxelMask> mask;
	glm::ivec3 chunkItr;
	glm::ivec3 axisMask;

	VoxelChunk() = default;

	explicit VoxelChunk(std::int32_t size, const glm::vec3& chunkPosition);

	vkx::Voxel at(const glm::ivec3& position);

	void set(const glm::ivec3& position, vkx::Voxel voxel);

	void set(std::int32_t x, std::int32_t y, std::int32_t z, vkx::Voxel voxel);

	void greedy();

private:
	bool validLocation(std::int32_t x, std::int32_t y, std::int32_t z);

	void computeMask(std::int32_t axis1, std::int32_t axis2);

	std::int32_t computeWidth(const vkx::VoxelMask& currentMask, std::int32_t i, std::int32_t n);

	std::int32_t computeHeight(const vkx::VoxelMask& currentMask, std::int32_t j, std::int32_t n, std::int32_t width);

	void clear(std::int32_t width, std::int32_t height, std::int32_t n);

	void computeMesh(std::int32_t axis1, std::int32_t axis2);

	void createQuad(std::int32_t normal, const glm::ivec3& axisMask, std::int32_t width, std::int32_t height, const glm::ivec3& pos, std::int32_t axis1, std::int32_t axis2);
};

struct VoxelVertex {
	glm::vec2 pos;
	glm::vec2 uv;
	glm::vec2 normal;

	VoxelVertex() = default;

	explicit VoxelVertex(const glm::vec2& pos, const glm::vec2& uv, const glm::vec2& normal);

	static auto getBindingDescription() noexcept {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};

		bindingDescriptions.push_back({0, sizeof(VoxelVertex), VK_VERTEX_INPUT_RATE_VERTEX});

		return bindingDescriptions;
	}

	static auto getAttributeDescriptions() noexcept {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VoxelVertex, pos)});
		attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VoxelVertex, uv)});
		attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VoxelVertex, normal)});

		return attributeDescriptions;
	}
};

static constexpr std::size_t CHUNK_SIZE = 4;

class VoxelChunk2D {
private:
	glm::vec2 position;
	std::vector<vkx::Voxel> voxels;
	std::vector<vkx::VoxelVertex> vertices;
	std::vector<std::uint32_t> indices;
	std::vector<vkx::VoxelVertex>::iterator vertexIter;
	std::vector<std::uint32_t>::iterator indexIter;
	std::int32_t vertexCount = 0;

public:
	explicit VoxelChunk2D(const glm::vec2& chunkPosition);

	void generateTerrain();

	void generateTest();

	[[nodiscard]] vkx::Mesh generateMesh(VmaAllocator allocator);

	[[nodiscard]] vkx::Voxel at(std::size_t i) const;

	void set(std::size_t i, vkx::Voxel voxel);

private:
	void createQuad(std::int32_t normal, const glm::vec2& axisMask, std::int32_t width, std::int32_t height, const glm::vec2& pos, std::int32_t axis1, std::int32_t axis2);
};
} // namespace vkx