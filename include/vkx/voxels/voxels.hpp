#pragma once

#include <algorithm>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <vkx/voxels/voxel_mask.hpp>
#include <vkx/renderer/core/vertex.hpp>
#include <vkx/voxels/voxel_matrix.hpp>
#include <vkx/camera.hpp>
#include <iostream>

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

	void printVec3(const glm::vec3 a) {
		std::cout << '[' << a.x << ',' << a.y << ',' << a.z << "]\n";
	}

	void raycast(const vkx::Camera& camera, std::int32_t width, std::int32_t height) {
		constexpr float nearZ = 0.1f;
		constexpr float rayLength = 4.0f;

		const auto rotationMatrix = glm::mat4_cast(glm::conjugate(camera.yawOrientation * camera.pitchOrientation));
		const auto startPosition = camera.position + glm::vec3(chunkPosition * 16);

		const auto center = glm::vec2(width / 2, height / 2);
		const auto transformedCenter = glm::mat2(rotationMatrix) * center;

		const auto localNormal = glm::vec3(((transformedCenter.x / width) * 2.0 - 1.0) * center.x, ((1.0 - (transformedCenter.y / height)) * 2.0 - 1.0) * center.y, -nearZ);
		const auto rayNormal = glm::vec3(rotationMatrix * glm::vec4(glm::normalize(localNormal), 1.0f));

		const auto endPosition = startPosition + rayNormal * rayLength;

		printVec3(startPosition);
		printVec3(endPosition);
		std::cout << "Distance=" << glm::distance(endPosition, startPosition) << '\n';

		
	}

	void greedy() {
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
				// mask.calculate(voxels, chunkItr, axis1, axis2, axisMask);
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

					glm::i32vec3 deltaAxis1{0};
					glm::i32vec3 deltaAxis2{0};
					deltaAxis1[axis1] = width;
					deltaAxis2[axis2] = height;

					createQuad(currentMask.normal, axisMask, width, height, chunkItr, axis1, axis2);

					// mask.clear(i, j, width, height);
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

	VoxelMatrix voxels;
	const glm::ivec3 chunkPosition = glm::vec3(0);
	const glm::ivec3 normalizedPosition = chunkPosition * static_cast<std::int32_t>(size);
};
} // namespace vkx