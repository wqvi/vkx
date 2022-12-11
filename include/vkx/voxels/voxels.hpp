#pragma once

#include <vkx/renderer/core/vertex.hpp>

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

class VoxelChunk2D {
public:
	static constexpr std::size_t SIZE = 16;

	explicit VoxelChunk2D(const glm::vec3& chunkPosition);

	void generateTerrain();

	void generateMesh();

	[[nodiscard]]
	Voxel at(std::size_t i) const;

	void set(std::size_t i, Voxel voxel);

private:
	std::vector<Voxel> voxels;
};
} // namespace vkx