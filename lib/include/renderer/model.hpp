#pragma once

#include <renderer/buffer.hpp>
#include <renderer/image.hpp>

namespace vkx
{
    class Mesh
    {
    public:
        Mesh() = default;

        explicit Mesh(std::vector<Vertex> const &vertices, std::vector<std::uint32_t> const &indices, Device const &device);

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

    private:
        Image image;
        vk::UniqueImageView view;
        vk::UniqueSampler sampler;
    };

    class Model
    {

    };
}