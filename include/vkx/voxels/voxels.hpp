#pragma once

#include "vkx/renderer/core/vertex.hpp"
#include <glm/fwd.hpp>
#include <vkx/renderer/core/renderer_base.hpp>
#include <vkx/voxels/greedy_mask.hpp>
#include <vkx/voxels/voxel_matrix.hpp>

namespace vkx {
class VoxelChunk {
public:
	explicit VoxelChunk(const glm::vec3& worldPosition, std::int32_t size)
	    : worldPosition(worldPosition), voxels(size, size, size), axisLimit(size) {

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
		for (std::int32_t axis = 0; axis < 3; axis++) {
			const auto axis1 = (axis + 1) % 3;
			const auto axis2 = (axis + 2) % 3;

			glm::i32vec3 chunkItr(0, 0, 0);
			glm::i32vec3 axisMask(0, 0, 0);

			axisMask[axis] = 1;

			GreedyMask mask{axisLimit, axisLimit};

			// Check each slice of the chunk
			for (chunkItr[axis] = -1; chunkItr[axis] < axisLimit;) {
				// Compute Mask
				mask.calculate(voxels, chunkItr, axis1, axis2, axisMask);

				chunkItr[axis]++;

				// Generate Mesh From Mask
				generateMesh(mask, axis1, axis2, axisMask, chunkItr);
			}
		}
	}

	std::vector<vkx::Vertex> vertices;
	std::vector<std::uint32_t> indices;

private:
	void generateMesh(GreedyMask& mask, std::int32_t axis1, std::int32_t axis2, const glm::i32vec3& axisMask, glm::i32vec3& chunkItr) {
		for (int j = 0; j < axisLimit; ++j) {
			for (int i = 0; i < axisLimit;) {
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

					// createQuad(
					//     currentMask, axisMask, width, height,
					//     chunkItr,
					//     chunkItr + deltaAxis1,
					//     chunkItr + deltaAxis2,
					//     chunkItr + deltaAxis1 + deltaAxis2);

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

		const auto v1 = vkx::Vertex{-pos, {0.0f, 0.0f}, -maskNormal};
		const auto v2 = vkx::Vertex{-(pos + deltaAxis), {width, 0.0f}, -maskNormal};
		deltaAxis[axis1] = 0;
		deltaAxis[axis2] = height;
		const auto v3 = vkx::Vertex{-(pos + deltaAxis), {0.0f, height}, -maskNormal};
		deltaAxis[axis1] = width;
		const auto v4 = vkx::Vertex{-(pos + deltaAxis), {width, height}, -maskNormal};

		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		vertices.push_back(v4);

		indices.push_back(vertexCount);
		indices.push_back(vertexCount + 2 + normal);
		indices.push_back(vertexCount + 2 - normal);
		indices.push_back(vertexCount + 3);
		indices.push_back(vertexCount + 1 - normal);
		indices.push_back(vertexCount + 1 + normal);

		vertexCount += 4;
	}

	void createQuad(
	    VoxelMask mask,
	    const glm::ivec3& axisMask,
	    const int width,
	    const int height,
	    const glm::ivec3& V1,
	    const glm::ivec3& V2,
	    const glm::ivec3& V3,
	    const glm::ivec3& V4) {
		const auto normal = axisMask * mask.normal;

		const auto v1 = vkx::Vertex{-V1, glm::vec2(0.0f, 0.0f), -normal};
		const auto v2 = vkx::Vertex{-V2, glm::vec2(width, 0.0f), -normal};
		const auto v3 = vkx::Vertex{-V3, glm::vec2(0.0f, height), -normal};
		const auto v4 = vkx::Vertex{-V4, glm::vec2(width, height), -normal};

		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		vertices.push_back(v4);

		indices.push_back(vertexCount);
		indices.push_back(vertexCount + 2 + mask.normal);
		indices.push_back(vertexCount + 2 - mask.normal);
		indices.push_back(vertexCount + 3);
		indices.push_back(vertexCount + 1 - mask.normal);
		indices.push_back(vertexCount + 1 + mask.normal);

		vertexCount += 4;
	}

	const std::int32_t axisLimit = 16;
	VoxelMatrix voxels;
	std::uint32_t vertexCount = 0;
	glm::vec3 worldPosition = glm::vec3(0);
};
} // namespace vkx