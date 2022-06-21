#pragma once

#include <renderer/image.hpp>

namespace vkx
{
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
}