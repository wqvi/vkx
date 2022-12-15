#include <glm/gtc/noise.hpp>
#include <vkx/voxels/voxels.hpp>

bool vkx::VoxelMask::operator==(const vkx::VoxelMask& other) const {
	return voxel == other.voxel && normal == other.normal;
}

bool vkx::VoxelMask::operator!=(const vkx::VoxelMask& other) const {
	return voxel != other.voxel && normal != other.normal;
}

vkx::VoxelChunk::VoxelChunk(std::int32_t size, const glm::vec3& chunkPosition)
    : size(size),
      chunkPosition(chunkPosition),
      voxels(size * size * size, vkx::Voxel::Stone),
      vertices(size * size * size * 4),
      indices(size * size * size * 6),
      vertexCount(0),
      vertexIter(vertices.begin()),
      indexIter(indices.begin()),
      normalizedPosition(glm::ivec3(chunkPosition) * size),
      mask(size * size),
      chunkItr(0),
      axisMask(0) {
	for (std::int32_t i = 0; i < size; i++) {
		if (i % 3 == 0) {
			set(i, 0, 0, vkx::Voxel::Air);
		}
	}

	for (std::int32_t i = 0; i < size; i++) {
		if (i % 3 == 0) {
			set(i, size - 1, 0, vkx::Voxel::Air);
		}
	}
}

vkx::Voxel vkx::VoxelChunk::at(const glm::ivec3& position) {
	if (!validLocation(position.x, position.y, position.z)) {
		return vkx::Voxel::Air;
	}

	return voxels[vkx::flattenIndex(size, position.x, position.y, position.z)];
}

void vkx::VoxelChunk::set(const glm::ivec3& position, vkx::Voxel voxel) {
	if (!validLocation(position.x, position.y, position.z)) {
		return;
	}

	voxels[vkx::flattenIndex(size, position.x, position.y, position.z)] = voxel;
}

void vkx::VoxelChunk::set(std::int32_t x, std::int32_t y, std::int32_t z, vkx::Voxel voxel) {
	if (!validLocation(x, y, z)) {
		return;
	}

	voxels[vkx::flattenIndex(size, x, y, z)] = voxel;
}

void vkx::VoxelChunk::greedy() {
	vertexCount = 0;
	vertexIter = vertices.begin();
	indexIter = indices.begin();

	for (std::int32_t axis = 0; axis < 3; axis++) {
		const auto axis1 = (axis + 1) % 3;
		const auto axis2 = (axis + 2) % 3;

		chunkItr = glm::ivec3{0};
		axisMask = glm::ivec3{0};

		axisMask[axis] = 1;
		chunkItr[axis] = -1;

		while (chunkItr[axis] < size) {
			computeMask(axis1, axis2);

			chunkItr[axis]++;

			computeMesh(axis1, axis2);
		}
	}
}

bool vkx::VoxelChunk::validLocation(std::int32_t x, std::int32_t y, std::int32_t z) {
	return x >= 0 && x < size && y >= 0 && y < size && z >= 0 && z < size;
}

void vkx::VoxelChunk::computeMask(std::int32_t axis1, std::int32_t axis2) {
	int n = 0;
	for (chunkItr[axis2] = 0; chunkItr[axis2] < size; chunkItr[axis2]++) {
		for (chunkItr[axis1] = 0; chunkItr[axis1] < size; chunkItr[axis1]++) {
			const auto currentVoxel = at(chunkItr);
			const auto compareVoxel = at(chunkItr + axisMask);

			const bool currentVoxelOpaque = currentVoxel != Voxel::Air;
			const bool compareVoxelOpaque = compareVoxel != Voxel::Air;

			if (currentVoxelOpaque == compareVoxelOpaque) {
				mask[n++] = VoxelMask{Voxel::Air, 0};
			} else if (currentVoxelOpaque) {
				mask[n++] = VoxelMask{currentVoxel, 1};
			} else {
				mask[n++] = VoxelMask{compareVoxel, -1};
			}
		}
	}
}

std::int32_t vkx::VoxelChunk::computeWidth(const vkx::VoxelMask& currentMask, std::int32_t i, std::int32_t n) {
	std::int32_t width = 1;

	while (i + width < size && mask[n + width] == currentMask) {
		width++;
	}

	return width;
}

std::int32_t vkx::VoxelChunk::computeHeight(const vkx::VoxelMask& currentMask, std::int32_t j, std::int32_t n, std::int32_t width) {
	std::int32_t height = 1;
	bool done = false;

	while (j + height < size) {
		for (std::int32_t k = 0; k < width; k++) {
			if (mask[n + k + height * size] == currentMask) {
				continue;
			}

			done = true;
			break;
		}

		if (done) {
			break;
		}

		height++;
	}

	return height;
}

void vkx::VoxelChunk::clear(std::int32_t width, std::int32_t height, std::int32_t n) {
	for (std::int32_t l = 0; l < height; l++) {
		for (std::int32_t k = 0; k < width; k++) {
			mask[n + k + l * size] = vkx::VoxelMask{vkx::Voxel::Air, 0};
		}
	}
}

void vkx::VoxelChunk::computeMesh(std::int32_t axis1, std::int32_t axis2) {
	std::int32_t n = 0;
	for (std::int32_t j = 0; j < size; j++) {
		for (std::int32_t i = 0; i < size;) {
			const auto& currentMask = mask[n];
			if (currentMask.normal != 0) {
				chunkItr[axis1] = i;
				chunkItr[axis2] = j;

				const auto width = computeWidth(currentMask, i, n);

				const auto height = computeHeight(currentMask, j, n, width);

				createQuad(currentMask.normal, axisMask, width, height, chunkItr, axis1, axis2);

				clear(width, height, n);

				i += width;
				n += width;
			} else {
				i++;
				n++;
			}
		}
	}
}

void vkx::VoxelChunk::createQuad(std::int32_t normal, const glm::ivec3& axisMask, std::int32_t width, std::int32_t height, const glm::ivec3& pos, std::int32_t axis1, std::int32_t axis2) {
	const auto maskNormal = axisMask * normal;

	glm::ivec3 deltaAxis{0, 0, 0};
	deltaAxis[axis1] = width;

	const auto v1 = vkx::Vertex{normalizedPosition + -pos, {0.0f, 0.0f}, -maskNormal};
	const auto v2 = vkx::Vertex{normalizedPosition + -(pos + deltaAxis), {width, 0.0f}, -maskNormal};
	deltaAxis[axis1] = 0;
	deltaAxis[axis2] = height;
	const auto v3 = vkx::Vertex{normalizedPosition + -(pos + deltaAxis), {0.0f, height}, -maskNormal};
	deltaAxis[axis1] = width;
	const auto v4 = vkx::Vertex{normalizedPosition + -(pos + deltaAxis), {width, height}, -maskNormal};

	*vertexIter = v1;
	vertexIter++;
	*vertexIter = v2;
	vertexIter++;
	*vertexIter = v3;
	vertexIter++;
	*vertexIter = v4;
	vertexIter++;

	*indexIter = vertexCount;
	indexIter++;
	*indexIter = vertexCount + 2 + normal;
	indexIter++;
	*indexIter = vertexCount + 2 - normal;
	indexIter++;
	*indexIter = vertexCount + 3;
	indexIter++;
	*indexIter = vertexCount + 1 - normal;
	indexIter++;
	*indexIter = vertexCount + 1 + normal;
	indexIter++;

	vertexCount += 4;
}

vkx::VoxelVertex::VoxelVertex(const glm::vec2& pos, const glm::vec2& uv, const glm::vec2& normal)
    : pos(pos), uv(uv), normal(normal) {}

vkx::VoxelChunk2D::VoxelChunk2D(const glm::vec2& chunkPosition)
    : position(chunkPosition) {
	voxels.resize(CHUNK_SIZE * CHUNK_SIZE);
	vertices.resize(CHUNK_SIZE * CHUNK_SIZE * 4);
	indices.resize(CHUNK_SIZE * CHUNK_SIZE * 6);
}

void vkx::VoxelChunk2D::generateTerrain() {
	for (std::size_t x = 0; x < CHUNK_SIZE; x++) {
		for (std::size_t y = 0; y < CHUNK_SIZE; y++) {
			const auto value = (glm::simplex(position + glm::vec2(x, y)) + 1.0f) / 2.0f;

			auto voxel = vkx::Voxel::Air;

			if (value < 0.5f) {
				voxel = vkx::Voxel::Stone;
			}

			voxels[x + y * CHUNK_SIZE] = voxel;
		}
	}
}

void vkx::VoxelChunk2D::generateTest() {
	for (std::size_t x = 0; x < CHUNK_SIZE; x++) {
		for (std::size_t y = 0; y < CHUNK_SIZE; y++) {
			voxels[x + y * CHUNK_SIZE] = vkx::Voxel::Stone;
		}
	}

	for (std::size_t x = 0; x < CHUNK_SIZE; x++) {
		voxels[x + 0 * CHUNK_SIZE] = vkx::Voxel::Air;
	}

	for (std::size_t i = 0; i < CHUNK_SIZE; i++) {
		voxels[0 + i * CHUNK_SIZE] = vkx::Voxel::Air;
	}

	voxels[0] = vkx::Voxel::Stone;
}

vkx::Mesh vkx::VoxelChunk2D::generateMesh(VmaAllocator allocator) {
	vertexIter = vertices.begin();
	indexIter = indices.begin();
	vertexCount = 0;

	std::vector<vkx::VoxelMask> mask{CHUNK_SIZE * CHUNK_SIZE};

	auto n = 0;
	for (auto y = 0; y < CHUNK_SIZE; y++) {
		for (auto x = 0; x < CHUNK_SIZE; x++) {
			const auto currentVoxel = at(x + y * CHUNK_SIZE);
			const auto compareVoxel = at((static_cast<std::size_t>(x) + 1) + y * CHUNK_SIZE);

			const auto currentVoxelOpaque = currentVoxel != vkx::Voxel::Air;
			const auto compareVoxelOpaque = compareVoxel != vkx::Voxel::Air;

			if (currentVoxelOpaque == compareVoxelOpaque) {
				mask[n++] = vkx::VoxelMask{vkx::Voxel::Air, 0};
			} else if (currentVoxelOpaque) {
				mask[n++] = vkx::VoxelMask{currentVoxel, 1};
			} else {
				mask[n++] = vkx::VoxelMask{compareVoxel, -1};
			}
		}
	}

	n = 0;
	for (int j = 0; j < CHUNK_SIZE; j++) {
		for (int i = 0; i < CHUNK_SIZE;) {
			if (mask[n].normal != 0) {
				const auto& currentMask = mask[n];

				auto width = 1;
				for (; i + width < CHUNK_SIZE && (mask[n + width] == currentMask); width++) {
				}

				auto height = 1;
				bool done = false;
				for (; j + height < CHUNK_SIZE; height++) {
					for (int k = 0; k < width; ++k) {
						if ((mask[n + k + height * CHUNK_SIZE] == currentMask)) {
							continue;
						}

						done = true;
						break;
					}

					if (done) {
						break;
					}
				}

				createQuad(currentMask.normal, width, height, {i, j});

				for (int l = 0; l < height; ++l) {
					for (int k = 0; k < width; ++k) {
						mask[n + k + l * CHUNK_SIZE] = vkx::VoxelMask{vkx::Voxel::Air, 0};
					}
				}

				i += width;
				n += width;
			} else {
				i++;
				n++;
			}
		}
	}

	vkx::Mesh voxelMesh{vertices.data(), sizeof(vkx::VoxelVertex) * vertices.size(), indices.data(), sizeof(std::uint32_t) * indices.size(), allocator};
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

// FIX ME YOU KNOW THE ISSUE
void vkx::VoxelChunk2D::createQuad(std::int32_t normal, std::int32_t width, std::int32_t height, const glm::vec2& pos) {
	const auto v2 = pos + glm::vec2{width, 0};
	const auto v3 = pos + glm::vec2{width, height};
	const auto v4 = pos + glm::vec2{0, height};

	if (normal == 1.0f || normal == -1.0f) {
		*vertexIter = vkx::VoxelVertex{pos, glm::vec2{width, height}, {}};
		vertexIter++;
		*vertexIter = vkx::VoxelVertex{v2, glm::vec2{0, height}, {}};
		vertexIter++;
		*vertexIter = vkx::VoxelVertex{v3, glm::vec2{width, 0}, {}};
		vertexIter++;
		*vertexIter = vkx::VoxelVertex{v4, glm::vec2{0, 0}, {}};
		vertexIter++;
	} else {
		*vertexIter = vkx::VoxelVertex{pos, glm::vec2{height, width}, {}};
		vertexIter++;
		*vertexIter = vkx::VoxelVertex{v2, glm::vec2{height, 0}, {}};
		vertexIter++;
		*vertexIter = vkx::VoxelVertex{v3, glm::vec2{0, width}, {}};
		vertexIter++;
		*vertexIter = vkx::VoxelVertex{v4, glm::vec2{0, 0}, {}};
		vertexIter++;
	}

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

	vertexCount += 4;
}