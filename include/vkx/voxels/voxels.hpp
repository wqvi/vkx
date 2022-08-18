#pragma once

#include "vkx/voxels/voxel_types.hpp"
#include <cmath>
#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <utility>
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

	auto raycast(const vkx::Camera& camera, std::int32_t width, std::int32_t height) {
		constexpr float rayLength = 4.0f;

		const auto rayOrigin = glm::vec3(normalizedPosition) - camera.position;

		const glm::vec3 rayDirection(0.0f, 0.0f, 1.0f);

		glm::ivec3 hitPos = glm::floor(rayOrigin);

		auto hitPrevPos = hitPos;

		const int xStep = rayDirection.x > 0 ? 1 : rayDirection.x < 0 ? -1 : 0;
		const int yStep = rayDirection.y > 0 ? 1 : rayDirection.y < 0 ? -1 : 0;
		const int zStep = rayDirection.z > 0 ? 1 : rayDirection.z < 0 ? -1 : 0;

		const auto xDelta = xStep != 0 ? 1.0f / glm::abs(rayDirection.x) : INFINITY;
		const auto yDelta = yStep != 0 ? 1.0f / glm::abs(rayDirection.y) : INFINITY;
		const auto zDelta = zStep != 0 ? 1.0f / glm::abs(rayDirection.z) : INFINITY;

		float xCross;
		float yCross;
		float zCross;

		if (xStep != 0) {
			if (xStep == 1) {
				xCross = (glm::ceil(rayOrigin.x) - rayOrigin.x) * xDelta;
			} else {
				xCross = (rayOrigin.x - glm::floor(rayOrigin.x)) * xDelta;
			}
		} else {
			xCross = INFINITY;
		}

		if (yStep != 0) {
			if (yStep == 1) {
				yCross = (glm::ceil(rayOrigin.y) - rayOrigin.y) * yDelta;
			} else {
				yCross = (rayOrigin.y - glm::floor(rayOrigin.y)) * yDelta;
			}
		} else {
			yCross = INFINITY;
		}

		if (zStep != 0) {
			if (zStep == 1) {
				zCross = (glm::ceil(rayOrigin.z) - rayOrigin.z) * zDelta;
			} else {
				zCross = (rayOrigin.z - glm::floor(rayOrigin.z)) * zDelta;
			}
		} else {
			zCross = INFINITY;
		}

		Voxel voxel = Voxel::Air;
		while (voxel == Voxel::Air) {
			hitPrevPos = hitPos;

			if (xCross < yCross) {
				if (xCross < zCross) {
					hitPos.x += xStep;

					if (xCross > rayLength) {
						return std::make_pair(false, glm::ivec3(0));
					}

					xCross += xDelta;
				} else {
					hitPos.z += zStep;
					
					if (zCross > rayLength) {
						return std::make_pair(false, glm::ivec3(0));
					}

					zCross += zDelta;
				}
			} else {
				if (yCross < zCross) {
					hitPos.y += yStep;

					if (yCross > rayLength) {
						return std::make_pair(false, glm::ivec3(0));
					}

					yCross += yDelta;
				} else {
					hitPos.z += zStep;
					
					if (zCross > rayLength) {
						return std::make_pair(false, glm::ivec3(0));
					}

					zCross += zDelta;
				}
			}

			voxel = voxels.at(hitPos);
		}

		return std::make_pair(true, hitPos);
	}

	void greedy() {
		vertexCount = 0;

		vertexIter = ve.begin();
		indexIter = in.begin();

		Mask mask;

		for (std::int32_t axis = 0; axis < 3; axis++) {
			const auto axis1 = (axis + 1) % 3;
			const auto axis2 = (axis + 2) % 3;

			glm::i32vec3 chunkItr(0, 0, 0);
			glm::i32vec3 axisMask(0, 0, 0);

			axisMask[axis] = 1;

			// Check each slice of the chunk
			for (chunkItr[axis] = -1; chunkItr[axis] < size;) {
				// Compute Mask
				calculateMask(mask, axis1, axis2, chunkItr, axisMask);

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

	VoxelMatrix voxels;
	glm::ivec3 chunkPosition = glm::vec3(0);
	glm::ivec3 normalizedPosition = chunkPosition * static_cast<std::int32_t>(size);

private:
	void calculateMask(Mask& mask, std::int32_t axis1, std::int32_t axis2, glm::ivec3& chunkItr, const glm::ivec3& axisMask) {
		for (chunkItr[axis2] = 0; chunkItr[axis2] < size; ++chunkItr[axis2]) {
			for (chunkItr[axis1] = 0; chunkItr[axis1] < size; ++chunkItr[axis1]) {
				const auto currentVoxel = voxels.at(chunkItr);
				const auto compareVoxel = voxels.at(chunkItr + axisMask);

				const bool currentVoxelOpaque = currentVoxel != Voxel::Air;
				const bool compareVoxelOpaque = compareVoxel != Voxel::Air;

				if (currentVoxelOpaque == compareVoxelOpaque) {
					mask[chunkItr[axis2] * size + chunkItr[axis1]] = VoxelMask(Voxel::Air, 0);
				} else if (currentVoxelOpaque) {
					mask[chunkItr[axis2] * size + chunkItr[axis1]] = VoxelMask(currentVoxel, 1);
				} else {
					mask[chunkItr[axis2] * size + chunkItr[axis1]] = VoxelMask(compareVoxel, -1);
				}
			}
		}
	}

	static void clear(Mask& mask, std::int32_t i, std::int32_t j, std::int32_t width, std::int32_t height) {
		for (std::int32_t l = 0; l < height; l++) {
			for (std::int32_t k = 0; k < width; k++) {
				const auto y = j + l;
				const auto x = i + k;
				const auto index = y * size + x;
				mask[index] = VoxelMask(Voxel::Air, 0);
			}
		}
	}

	static auto calculateQuadWidth(const Mask& mask, const VoxelMask& currentMask, std::int32_t i, std::int32_t j) {
		std::int32_t width = 1;
		while (i + width < size) {
			const auto& quadMask = mask[(j * size + i) + width];
			if (quadMask != currentMask) {
				return width;
			}
			width++;
		}
		return width;
	}

	static auto calculateQuadHeight(const Mask& mask, const VoxelMask& currentMask, std::int32_t quadWidth, std::int32_t i, std::int32_t j) {
		std::int32_t height = 1;
		while (j + height < size) {
			for (std::int32_t k = 0; k < quadWidth; k++) {
				const auto index = j * size + i;
				const auto offset = k + height * size;
				const auto& quadMask = mask[index + offset];
				if (quadMask != currentMask) {
					return height;
				}
			}
			height++;
		}
		return height;
	}

	void generateMesh(Mask& mask, std::int32_t axis1, std::int32_t axis2, const glm::i32vec3& axisMask, glm::i32vec3& chunkItr) {
		for (int j = 0; j < size; ++j) {
			for (int i = 0; i < size;) {
				const auto& currentMask = mask[j * size + i];
				if (currentMask.normal != 0) {
					chunkItr[axis1] = i;
					chunkItr[axis2] = j;

					const auto width = calculateQuadWidth(mask, currentMask, i, j);
					const auto height = calculateQuadHeight(mask, currentMask, width, i, j);

					createQuad(currentMask.normal, axisMask, width, height, chunkItr, axis1, axis2);

					clear(mask, i, j, width, height);

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
};
} // namespace vkx