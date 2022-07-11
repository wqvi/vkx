#pragma once

#include <vkx/renderer/buffer.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/uniform_buffer.hpp>
#include <vkx/node.hpp>

namespace vkx {
    class Mesh {
    public:
        Mesh() = default;

        explicit Mesh(std::vector<Vertex> const &vertices, std::vector<std::uint32_t> const &indices, Device const &device);

        VertexBuffer vertexBuffer;
        IndexBuffer indexBuffer;
        std::size_t indexCount = 0;
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
        Material material = {};
        glm::vec3 position = glm::vec3(0);
    };
}