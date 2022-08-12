#pragma once

#include "vkx/renderer/core/vertex.hpp"
#include <SDL2/SDL_log.h>
#include <array>
#include <cstdint>
#include <vkx/renderer/core/renderer_base.hpp>
#include <vkx/voxels/greedy_mask.hpp>
#include <vkx/voxels/voxel_matrix.hpp>

namespace vkx {
template <std::int32_t size>
class VoxelChunk {
public:
	static_assert(size % 8 == 0, "Size must be a multiple of 8");

	explicit VoxelChunk(const glm::vec3& chunkPosition)
	    : chunkPosition(chunkPosition), voxels(size, size, size) {

		for (std::int32_t x = 0; x < size; x++) {
			for (std::int32_t y = 0; y < size; y++) {
				for (std::int32_t z = 0; z < size / 2; z++) {
					voxels.set(x, y, z, Voxel{VoxelType::Stone});
				}
			}
		}

		for (std::int32_t i = 0; i < size; i++) {
			voxels.set(i, 2, 2, Voxel{VoxelType::Stone});
		}

		for (std::int32_t i = 0; i < size; i++) {
			voxels.set(0, 0, i, Voxel{VoxelType::Stone});
		}

		for (std::int32_t i = 0; i < size; i++) {
			for (std::int32_t j = size - size / 2; j < size; j++) {
				voxels.set(0, j, i, Voxel{VoxelType::Stone});
			}
		}

		for (std::int32_t i = 0; i < size; i++) {
			voxels.set(i, 0, 0, Voxel{VoxelType::Air, false});
		}

		voxels.set(1, 1, 1, Voxel(VoxelType::Stone));
	}

	void greedy() {
		vertexIter = ve.begin();
		indexIter = in.begin();

		GreedyMask mask{size, size};

		for (std::int32_t axis = 0; axis < 3; axis++) {
			const auto axis1 = (axis + 1) % 3;
			const auto axis2 = (axis + 2) % 3;

			glm::i32vec3 chunkItr(0, 0, 0);
			glm::i32vec3 axisMask(0, 0, 0);

			axisMask[axis] = 1;

			// Check each slice of the chunk
			for (chunkItr[axis] = -1; chunkItr[axis] < size;) {
				// Compute Mask
				mask.calculate(voxels, chunkItr, axis1, axis2, axisMask);

				chunkItr[axis]++;

				// Generate Mesh From Mask
				generateMesh(mask, axis1, axis2, axisMask, chunkItr);
			}
		}
	}

	std::array<vkx::Vertex, size * size * size * 4> ve;
	std::array<std::uint32_t, size * size * size * 6> in;

	std::uint32_t vertexCount = 0;

	vkx::Vertex* vertexIter;
	std::uint32_t* indexIter;

private:
	void generateMesh(GreedyMask& mask, std::int32_t axis1, std::int32_t axis2, const glm::i32vec3& axisMask, glm::i32vec3& chunkItr) {
		for (int j = 0; j < size; ++j) {
			for (int i = 0; i < size;) {
				const auto& currentMask = mask.at(i, j);
				if (currentMask.normal != 0) {
					chunkItr[axis1] = i;
					chunkItr[axis2] = j;

					const auto width = mask.calculateQuadWidth(currentMask, i, j);
					const auto height = mask.calculateQuadHeight(currentMask, width, i, j);

					glm::i32vec3 deltaAxis1{0};
					glm::i32vec3 deltaAxis2{0};
					deltaAxis1[axis1] = width;
					deltaAxis2[axis2] = height;

					createQuad(currentMask.normal, axisMask, width, height, chunkItr, axis1, axis2);

					mask.clear(i, j, width, height);

					i += width;
				} else {
					i++;
				}
			}
		}
	}

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

	VoxelMatrix voxels;
	glm::ivec3 chunkPosition = glm::vec3(0);
	const glm::ivec3 normalizedPosition = chunkPosition * static_cast<std::int32_t>(size);
};
} // namespace vkx