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

static constexpr std::size_t CHUNK_SIZE = 64;

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

	[[nodiscard]] vkx::Mesh generateMesh(VmaAllocator allocator);

	[[nodiscard]] vkx::Voxel at(std::size_t i) const;

	void set(std::size_t i, vkx::Voxel voxel);

private:
	void createQuad(std::int32_t width, std::int32_t height, const glm::vec2& pos);
};

class ChunkLoader {
public:
	ChunkLoader() = default;
};
} // namespace vkx