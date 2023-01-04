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

	VoxelMask() = default;

	explicit VoxelMask(Voxel voxel, std::int32_t normal);

	bool operator==(const VoxelMask& other) const;

	bool operator!=(const VoxelMask& other) const;
};

static constexpr std::size_t CHUNK_SIZE = 64;

static constexpr float CHUNK_RADIUS = 3.0f;

static constexpr float CHUNK_HALF_RADIUS = CHUNK_RADIUS / 2.0f;

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

	void generateTestBox();

	void generateMesh(vkx::Mesh& mesh);

	[[nodiscard]] vkx::Voxel at(std::size_t i) const;

	void set(std::size_t i, vkx::Voxel voxel);

	[[nodiscard]] glm::vec2 chunkPosition() const noexcept;

	[[nodiscard]] glm::vec2 globalPosition() const noexcept;

	void setGlobalPosition(const glm::vec2& globalPosition) noexcept;

private:
	std::uint32_t createQuad(std::vector<vkx::Vertex>::iterator vertexIter, std::vector<std::uint32_t>::iterator indexIter, std::uint32_t vertexCount, std::int32_t width, std::int32_t height, const glm::vec2& pos) const;
};

class ChunkLoader {
public:
	ChunkLoader() = default;
};
} // namespace vkx