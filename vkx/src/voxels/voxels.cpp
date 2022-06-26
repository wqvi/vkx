#include "voxels/voxels.hpp"

#include <iostream>

namespace vkx {
    VoxelChunk::VoxelChunk(const glm::vec3 &worldPosition, std::int32_t width, std::int32_t height, std::int32_t depth)
            : voxels(width, height, depth), worldPosition(worldPosition) {
        for (std::int32_t x = 0; x < width; x++) {
            for (std::int32_t y = 0; y < height; y++) {
                for (std::int32_t z = 0; z < depth / 2; z++) {
                    voxels.set(x, y, z, Voxel{VoxelType::Stone});
                }
            }
        }

        for (std::int32_t i = 0; i < width; i++) {
            voxels.set(i, 2, 2, Voxel{VoxelType::Stone});
        }

        for (std::int32_t i = 0; i < depth; i++) {
            voxels.set(0, 0, i, Voxel{VoxelType::Stone});
        }

        for (std::int32_t i = 0; i < depth; i++) {
            for (std::int32_t j = height - height / 2; j < height; j++) {
                voxels.set(0, j, i, Voxel{VoxelType::Stone});
            }
        }

        for (std::int32_t i = 0; i < width; i++) {
            voxels.set(i, 0, 0, Voxel{VoxelType::Air, false});
        }

        voxels.set(1, 1, 1, Voxel(VoxelType::Stone));
    }

    VoxelChunk::VoxelChunk(const glm::vec3 &worldPosition, std::int32_t size)
            : VoxelChunk(worldPosition, size, size, size) {}

    void VoxelChunk::greedy() {
        for (std::int32_t Axis = 0; Axis < 3; ++Axis) {
            auto const Axis1 = (Axis + 1) % 3;
            auto const Axis2 = (Axis + 2) % 3;

            auto const MainAxisLimit = voxels.getDimensionSize(Axis);
            auto const Axis1Limit = voxels.getDimensionSize(Axis1);
            auto const Axis2Limit = voxels.getDimensionSize(Axis2);


            glm::i32vec3 ChunkItr{0};
            glm::i32vec3 AxisMask{0};
            // glm::i32vec3 testTmp{0};

            AxisMask[Axis] = 1;

            GreedyMask Mask{Axis1Limit, Axis2Limit};

            // Check each slice of the chunk
            // testTmp[Axis] = -1;
            for (ChunkItr[Axis] = -1; ChunkItr[Axis] < MainAxisLimit;) {
                // Compute Mask
                Mask.calculate(voxels, ChunkItr, Axis1, Axis2, AxisMask);

                // testTmp[Axis]++;
                ChunkItr[Axis]++;

                // Generate Mesh From Mask
                generateMesh(Mask, Axis1, Axis2, AxisMask, ChunkItr);
            }
        }
    }

    static void printVec(const glm::vec3 &vec) {
        std::cout << '(' << vec.x << ',' << vec.y << ',' << vec.z << ")\n";
    }

    static glm::vec3 test2(const glm::vec3 &position, const glm::vec3 &voxelPosition) {
        const bool upperBound = position.x < voxelPosition.x - 1.0f &&
                                position.y < voxelPosition.y - 1.0f &&
                                position.z < voxelPosition.z - 1.0f;
        const bool lowerBound = position.x > voxelPosition.x + 1.0f &&
                                position.y > voxelPosition.y + 1.0f &&
                                position.z > voxelPosition.z + 1.0f;
        if (upperBound && lowerBound) {
            return voxelPosition - 1.0f;
        }
        printVec(position);
        printVec(voxelPosition + glm::vec3(1));
        printVec(voxelPosition - glm::vec3(1));
        std::cout << "Upper bound " << upperBound << "\t lower bound " << lowerBound << '\n';

        return position;
    }

    glm::vec3 VoxelChunk::test(const glm::vec3 &position) {
        // Due to how the mesh is rendered the world position is going to be subtracted from
        const bool upperBound = position.x > worldPosition.x - static_cast<float>(voxels.getWidth()) &&
                                position.y > worldPosition.y - static_cast<float>(voxels.getHeight()) &&
                                position.z > worldPosition.z - static_cast<float>(voxels.getDepth());
        // Continued here the position must be less than the world position due to the end of the mesh is far lower.
        const bool lowerBound = position.x < worldPosition.x &&
                                position.y < worldPosition.y &&
                                position.z < worldPosition.z;

        if (upperBound && lowerBound) {
            const auto voxelPosition = worldPosition - position;
            const auto &voxel = voxels.at(voxelPosition);
            if (voxel.collision) {
                return test2(position, voxelPosition);
            }
        }

        return position;
    }

    void
    VoxelChunk::generateMesh(GreedyMask &Mask, std::int32_t Axis1, std::int32_t Axis2, glm::i32vec3 const &AxisMask,
                             glm::i32vec3 &ChunkItr) {

        for (int j = 0; j < Mask.getHeight(); ++j) {
            for (int i = 0; i < Mask.getWidth();) {
                auto const &CurrentMask = Mask.at(i, j);
                if (CurrentMask.normal != 0) {
                    ChunkItr[Axis1] = i;
                    ChunkItr[Axis2] = j;

                    auto Width = Mask.calculateQuadWidth(CurrentMask, i, j);
                    auto Height = Mask.calculateQuadHeight(CurrentMask, Width, i, j);

                    glm::i32vec3 DeltaAxis1{0};
                    glm::i32vec3 DeltaAxis2{0};
                    DeltaAxis1[Axis1] = Width;
                    DeltaAxis2[Axis2] = Height;

                    CreateQuad(
                            CurrentMask, AxisMask, Width, Height,
                            ChunkItr,
                            ChunkItr + DeltaAxis1,
                            ChunkItr + DeltaAxis2,
                            ChunkItr + DeltaAxis1 + DeltaAxis2);

                    Mask.clear(i, j, Width, Height);

                    i += Width;
                } else {
                    i++;
                }
            }
        }
    }

    void VoxelChunk::CreateQuad(
            VoxelMask mask,
            const glm::ivec3 &axisMask,
            const int width,
            const int height,
            const glm::ivec3 &V1,
            const glm::ivec3 &V2,
            const glm::ivec3 &V3,
            const glm::ivec3 &V4) {
        auto normal = axisMask * mask.normal;

        auto v1 = vkx::Vertex{-V1, glm::vec2(0.0f, 0.0f), -normal};
        auto v2 = vkx::Vertex{-V2, glm::vec2(width, 0.0f), -normal};
        auto v3 = vkx::Vertex{-V3, glm::vec2(0.0f, height), -normal};
        auto v4 = vkx::Vertex{-V4, glm::vec2(width, height), -normal};

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
}