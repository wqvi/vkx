#pragma once

#include <renderer/buffer.hpp>
#include <renderer/image.hpp>
#include <renderer/uniform_buffer.hpp>

namespace vkx
{
    class Mesh
    {
    public:
        Mesh() = default;

        Mesh(std::vector<Vertex> const &vertices, std::vector<std::uint32_t> const &indices, Device const &device);

        explicit Mesh(std::size_t vertexCount, std::size_t indexCount, Device const &device);

    private:
        Buffer vertexBuffer;
        Buffer indexBuffer;
    };

    class Texture
    {
    public:
        Texture() = default;

        Texture(std::string const &file, Device const &device);

        [[nodiscard]] vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet const &descriptorSet, std::uint32_t dstBinding) const;
    private:
        Image image;
        vk::UniqueImageView view;
        vk::UniqueSampler sampler;
    };

    struct Model
    {
        explicit Model(Mesh &&mesh, Texture &&texture, const Material &material);

        Mesh mesh;
        Texture texture;
        Material material;
    };
}