#include <vkx/voxels/voxels.hpp>

vkx::VoxelMask::VoxelMask(Voxel voxel, std::int32_t normal) 
	: voxel(voxel), normal(normal) {
}

bool vkx::VoxelMask::operator==(const vkx::VoxelMask& other) const {
	return voxel == other.voxel && normal == other.normal;
}

bool vkx::VoxelMask::operator!=(const vkx::VoxelMask& other) const {
	return voxel != other.voxel || normal != other.normal;
}

vkx::VoxelChunk2D::VoxelChunk2D(const glm::vec2& chunkPosition)
    : globalPosition(chunkPosition * static_cast<float>(vkx::CHUNK_SIZE)) {
	voxels.resize(CHUNK_SIZE * CHUNK_SIZE);
}

void vkx::VoxelChunk2D::generateTerrain() {
	for (std::size_t x = 0; x < CHUNK_SIZE; x++) {
		for (std::size_t y = 0; y < CHUNK_SIZE; y++) {
			const auto global = globalPosition + glm::vec2(x, y);
			const auto height = (glm::simplex(global) + 1.0f) / 2.0f;

			auto voxel = vkx::Voxel::Air;

			if (height < 0.5f) {
				voxel = vkx::Voxel::Stone;
			}

			voxels[x + y * CHUNK_SIZE] = voxel;
		}
	}
}

void vkx::VoxelChunk2D::generateTestBox() {
	for (std::size_t x = 0; x < CHUNK_SIZE; x++) {
		for (std::size_t y = 0; y < CHUNK_SIZE; y++) {
			if (x > CHUNK_SIZE / 2) {
				voxels[x + y * CHUNK_SIZE] = vkx::Voxel::Stone;
			} else {
				voxels[x + y * CHUNK_SIZE] = vkx::Voxel::Dirt;
			}
		}
	}
}

void vkx::VoxelChunk2D::generateMesh(vkx::Mesh& mesh, vkx::Buffer& vertexBuffer, std::vector<std::uint32_t>::iterator iter) {
	std::vector<vkx::Vertex> vertices{vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 4};
	auto vertexIter = vertices.begin();
	auto indexIter = iter;
	auto vertexCount = 0;

	std::vector<vkx::VoxelMask> voxelMask{};
	voxelMask.reserve(CHUNK_SIZE * CHUNK_SIZE);
	for (auto i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++) {
		voxelMask.emplace_back(voxels[i], voxels[i] != vkx::Voxel::Air);
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

	mesh.activeIndexCount = std::distance(iter, indexIter);
	mesh.vertexBuffer.mapMemory(vertices.data());
	mesh.indexBuffer.mapMemory(&*iter);
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

std::uint32_t vkx::VoxelChunk2D::createQuad(std::vector<vkx::Vertex>::iterator vertexIter, std::vector<std::uint32_t>::iterator indexIter, std::uint32_t vertexCount, std::int32_t width, std::int32_t height, const glm::vec2& pos) const {
	const auto v1 = (globalPosition * 16.0f) + (pos) * 16.0f;
	const auto v2 = (globalPosition * 16.0f) + (pos + glm::vec2{width, 0}) * 16.0f;
	const auto v3 = (globalPosition * 16.0f) + (pos + glm::vec2{width, height}) * 16.0f;
	const auto v4 = (globalPosition * 16.0f) + (pos + glm::vec2{0, height}) * 16.0f;

	*vertexIter = vkx::Vertex{v1, glm::vec2{0, 0}, {}};
	vertexIter++;
	*vertexIter = vkx::Vertex{v2, glm::vec2{width, 0}, {}};
	vertexIter++;
	*vertexIter = vkx::Vertex{v3, glm::vec2{width, height}, {}};
	vertexIter++;
	*vertexIter = vkx::Vertex{v4, glm::vec2{0, height}, {}};
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