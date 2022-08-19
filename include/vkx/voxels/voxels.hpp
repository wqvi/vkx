#pragma once

#include "vkx/voxels/voxel_types.hpp"
#include <glm/fwd.hpp>
#include <iostream>
#include <vkx/camera.hpp>
#include <vkx/renderer/core/vertex.hpp>
#include <vkx/voxels/voxel_mask.hpp>
#include <vkx/voxels/voxel_matrix.hpp>

namespace vkx {
template <std::int32_t size>
class VoxelChunk {
	using Mask = std::array<VoxelMask, size * size>;

public:
	static_assert(size % 8 == 0, "Size must be a multiple of 8");
	explicit VoxelChunk(const glm::vec3& chunkPosition)
	    : chunkPosition(chunkPosition), voxels(size, size, size) {
		std::fill(voxels.begin(), voxels.end(), Voxel::Stone);
		for (std::int32_t i = 0; i < size; i++) {
			if (i % 3 == 0) {
				voxels.set(i, 0, 0, Voxel::Air);
			}
		}

		for (std::int32_t i = 0; i < size; i++) {
			if (i % 3 == 0) {
				voxels.set(i, size - 1, 0, Voxel::Air);
			}
		}
	}

	void computeMask(Mask& mask, glm::ivec3& chunkItr, const glm::ivec3& axisMask, int axis1, int axis2) {
		int n = 0;
		for (chunkItr[axis2] = 0; chunkItr[axis2] < size; chunkItr[axis2]++) {
			for (chunkItr[axis1] = 0; chunkItr[axis1] < size; ++chunkItr[axis1]) {
				const auto currentVoxel = voxels.at(chunkItr);
				const auto compareVoxel = voxels.at(chunkItr + axisMask);

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

	auto computeWidth(const Mask& mask, const VoxelMask& currentMask, int i, int n) {
		int width = 1;

		while (i + width < size && mask[n + width] == currentMask) {
			width++;
		}

		return width;
	}

	auto computeHeight(const Mask& mask, const VoxelMask& currentMask, int j, int n, int width) {
		int height = 1;
		bool done = false;

		while (j + height < size) {
			for (int k = 0; k < width; k++) {
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

	void clear(Mask& mask, int width, int height, int n) {
		for (int l = 0; l < height; ++l) {
			for (int k = 0; k < width; ++k) {
				mask[n + k + l * size] = VoxelMask{Voxel::Air, 0};
			}
		}
	}

	void computeMesh(Mask& mask, glm::ivec3& chunkItr, const glm::ivec3& axisMask, int axis1, int axis2) {
		int n = 0;
		for (int j = 0; j < size; ++j) {
			for (int i = 0; i < size;) {
				const auto& currentMask = mask[n];
				if (currentMask.normal != 0) {
					chunkItr[axis1] = i;
					chunkItr[axis2] = j;

					const int width = computeWidth(mask, currentMask, i, n);

					const int height = computeHeight(mask, currentMask, j, n, width);

					createQuad(currentMask.normal, axisMask, width, height, chunkItr, axis1, axis2);

					clear(mask, width, height, n);

					i += width;
					n += width;
				} else {
					i++;
					n++;
				}
			}
		}
	}

	void greedy() {
		vertexCount = 0;

		vertexIter = ve.begin();
		indexIter = in.begin();

		Mask mask;

		for (int axis = 0; axis < 3; ++axis) {
			const int axis1 = (axis + 1) % 3;
			const int axis2 = (axis + 2) % 3;

			glm::ivec3 chunkItr{0};
			glm::ivec3 axisMask{0};

			axisMask[axis] = 1;
			chunkItr[axis] = -1;

			while (chunkItr[axis] < size) {
				computeMask(mask, chunkItr, axisMask, axis1, axis2);

				chunkItr[axis]++;

				computeMesh(mask, chunkItr, axisMask, axis1, axis2);
			}
		}
	}

	std::array<vkx::Vertex, size * size * size * 4> ve;
	std::array<std::uint32_t, size * size * size * 6> in;

	std::uint32_t vertexCount = 0;

	vkx::Vertex* vertexIter;
	std::uint32_t* indexIter;

	VoxelMatrix voxels;
	glm::ivec3 chunkPosition = glm::vec3(0);
	glm::ivec3 normalizedPosition = chunkPosition * static_cast<std::int32_t>(size);

private:
	void createQuad(int normal, const glm::ivec3& axisMask, int width, int height, const glm::ivec3& pos, std::int32_t axis1, std::int32_t axis2) {
		const auto maskNormal = axisMask * normal;

		glm::ivec3 deltaAxis(0, 0, 0);
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
};
} // namespace vkx