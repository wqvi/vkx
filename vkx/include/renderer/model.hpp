#pragma once

#include <renderer/buffer.hpp>
#include <renderer/image.hpp>
#include <renderer/uniform_buffer.hpp>
#include <node.hpp>

namespace vkx {
    class Mesh {
    public:
        Mesh() = default;

        Mesh(std::vector<Vertex> const &vertices, std::vector<std::uint32_t> const &indices, Device const &device);

        explicit Mesh(std::size_t vertexCount, std::size_t indexCount, Device const &device);

        Buffer vertexBuffer;
        Buffer indexBuffer;
        std::size_t indexCount;
    };

    class Texture {
    public:
        Texture() = default;

        Texture(std::string const &file, Device const &device);

        [[nodiscard]]
        vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet const &descriptorSet,
                                                        std::uint32_t dstBinding) const;

    private:
        Image image;
        vk::UniqueImageView view;
        vk::UniqueSampler sampler;
    };

    struct Model : public Node {
        Model() = default;

        explicit Model(Mesh &&mesh, Texture &&texture, const Material &material);

        [[nodiscard]]
        glm::mat4 getModelMatrix() const noexcept;

        Mesh mesh;
        Texture texture;
        Material material;
        glm::vec3 position = glm::vec3(0);
    };
}