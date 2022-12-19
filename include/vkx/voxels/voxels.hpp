#pragma once

#include <vkx/renderer/core/vertex.hpp>
#include <vkx/renderer/model.hpp>

namespace vkx {
enum class Voxel : std::int32_t {
	Air,
	Stone,
	Dirt
};

struct VoxelMask {
	Voxel voxel = Voxel::Air;
	std::int32_t normal = 0;

	bool operator==(const VoxelMask& other) const;

	bool operator!=(const VoxelMask& other) const;
};

static constexpr std::size_t CHUNK_SIZE = 64;

class HostHeapMesh {
	friend class vkx::Mesh;

private:
	std::vector<vkx::Vertex> vertices;
	std::vector<std::uint32_t> indices;
	std::vector<vkx::Vertex>::iterator vertexIter;
	std::vector<std::uint32_t>::iterator indexIter;
	std::size_t vertexCount = 0;
	std::size_t indexCount = 0;

public:
	explicit HostHeapMesh(std::size_t maximumVertexCount, std::size_t maximumIndexCount);
};

class VoxelChunk2D {
private:
	glm::vec2 position;
	std::vector<vkx::Voxel> voxels;
	std::vector<vkx::Vertex> vertices;
	std::vector<std::uint32_t> indices;
	std::vector<vkx::Vertex>::iterator vertexIter;
	std::vector<std::uint32_t>::iterator indexIter;
	std::int32_t vertexCount = 0;

public:
	explicit VoxelChunk2D(const glm::vec2& chunkPosition);

	void generateTerrain();

	[[nodiscard]] vkx::Mesh generateMesh(VmaAllocator allocator);

	[[nodiscard]] vkx::Voxel at(std::size_t i) const;

	void set(std::size_t i, vkx::Voxel voxel);

private:
	static std::uint32_t createQuad(std::vector<vkx::Vertex>::iterator vertexIter, std::vector<std::uint32_t>::iterator indexIter, std::uint32_t vertexCount, std::int32_t width, std::int32_t height, const glm::vec2& pos);
};

class ChunkLoader {
public:
	ChunkLoader() = default;
};
} // namespace vkx