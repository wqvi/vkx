#include <vkx/voxels/voxels.hpp>

bool vkx::VoxelMask::operator==(const vkx::VoxelMask& other) const {
	return voxel == other.voxel && normal == other.normal;
}

bool vkx::VoxelMask::operator!=(const vkx::VoxelMask& other) const {
	return voxel != other.voxel || normal != other.normal;
}

vkx::VoxelChunk2D::VoxelChunk2D(const glm::vec2& chunkPosition)
    : position(chunkPosition) {
	voxels.resize(CHUNK_SIZE * CHUNK_SIZE);
	vertices.resize(CHUNK_SIZE * CHUNK_SIZE * 4);
	indices.resize(CHUNK_SIZE * CHUNK_SIZE * 6);
}

void vkx::VoxelChunk2D::generateTerrain() {
	for (std::size_t x = 0; x < CHUNK_SIZE; x++) {
		for (std::size_t y = 0; y < CHUNK_SIZE; y++) {
			const auto global = position * static_cast<float>(CHUNK_SIZE) + glm::vec2(x, y);
			const auto height = static_cast<std::uint32_t>((glm::simplex(global) + 1) / 2 * CHUNK_SIZE);

			auto voxel = vkx::Voxel::Air;

			if (y < height) {
				voxel = vkx::Voxel::Stone;
			}

			voxels[x + y * CHUNK_SIZE] = voxel;
		}
	}
}

vkx::Mesh vkx::VoxelChunk2D::generateMesh(VmaAllocator allocator) {
	vertexIter = vertices.begin();
	indexIter = indices.begin();
	vertexCount = 0;

	std::vector<vkx::VoxelMask> voxelMask{CHUNK_SIZE * CHUNK_SIZE};
	for (auto i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++) {
		voxelMask[i] = vkx::VoxelMask{voxels[i], voxels[i] != vkx::Voxel::Air};
	}

	auto n = 0;
	for (auto y = 0; y < CHUNK_SIZE; y++) {
		for (auto x = 0; x < CHUNK_SIZE;) {
			if (voxelMask[n].normal != 0) {
				const auto& currentMask = voxelMask[n];

				auto width = 1;
				for (; x + width < CHUNK_SIZE && voxelMask[n + width] == currentMask; width++) {
				}

				auto height = 1;
				auto done = false;
				for (; y + height < CHUNK_SIZE; height++) {
					for (auto i = 0; i < width; i++) {
						if (voxelMask[n + i + height * CHUNK_SIZE] != currentMask) {
							done = true;
							break;
						}
					}

					if (done) {
						break;
					}
				}

				vertexCount = createQuad(vertexIter, indexIter, vertexCount, width, height, {x, y});
				std::advance(vertexIter, 4);
				std::advance(indexIter, 6);

				for (auto j = 0; j < height; j++) {
					for (auto i = 0; i < width; i++) {
						voxelMask[n + i + j * CHUNK_SIZE] = vkx::VoxelMask{vkx::Voxel::Air, false};
					}
				}

				x += width;
				n += width;
			} else {
				x++;
				n++;
			}
		}
	}

	vkx::Mesh voxelMesh{vertices.data(), sizeof(vkx::Vertex) * vertices.size(), indices.data(), sizeof(std::uint32_t) * indices.size(), allocator};
	voxelMesh.indexCount = static_cast<std::size_t>(std::distance(indices.begin(), indexIter));

	return voxelMesh;
}

vkx::Voxel vkx::VoxelChunk2D::at(std::size_t i) const {
	if (i >= 0 && i < CHUNK_SIZE * CHUNK_SIZE) {
		return voxels[i];
	}

	return vkx::Voxel::Air;
}

void vkx::VoxelChunk2D::set(std::size_t i, vkx::Voxel voxel) {
	if (i >= 0 && i < CHUNK_SIZE * CHUNK_SIZE) {
		voxels[i] = voxel;
	}
}

std::uint32_t vkx::VoxelChunk2D::createQuad(std::vector<vkx::Vertex>::iterator vertexIter, std::vector<std::uint32_t>::iterator indexIter, std::uint32_t vertexCount, std::int32_t width, std::int32_t height, const glm::vec2& pos) {
	const auto v2 = pos + glm::vec2{width, 0};
	const auto v3 = pos + glm::vec2{width, height};
	const auto v4 = pos + glm::vec2{0, height};

	*vertexIter = vkx::Vertex{pos * 16.0f, glm::vec2{0, 0}, {}};
	vertexIter++;
	*vertexIter = vkx::Vertex{v2 * 16.0f, glm::vec2{width, 0}, {}};
	vertexIter++;
	*vertexIter = vkx::Vertex{v3 * 16.0f, glm::vec2{width, height}, {}};
	vertexIter++;
	*vertexIter = vkx::Vertex{v4 * 16.0f, glm::vec2{0, height}, {}};
	vertexIter++;

	*indexIter = vertexCount;
	indexIter++;
	*indexIter = vertexCount + 1;
	indexIter++;
	*indexIter = vertexCount + 2;
	indexIter++;
	*indexIter = vertexCount + 2;
	indexIter++;
	*indexIter = vertexCount + 3;
	indexIter++;
	*indexIter = vertexCount;
	indexIter++;

	return vertexCount + 4;
}

vkx::HostHeapMesh::HostHeapMesh(std::size_t maximumVertexCount, std::size_t maximumIndexCount) {
	vertices.resize(maximumVertexCount);
	indices.resize(maximumIndexCount);
	vertexIter = vertices.begin();
	indexIter = indices.begin();
}
