#include <renderer/texture.hpp>
#include <renderer/core/device.hpp>

namespace vkx
{
    Texture::Texture(std::string const &file, Device const &device)
        : image(file, device),
          view(device.createTextureImageViewUnique(static_cast<vk::Image>(image))),
          sampler(device.createTextureSamplerUnique())
    {
    }

    vk::WriteDescriptorSet Texture::createWriteDescriptorSet(vk::DescriptorSet const &descriptorSet, std::uint32_t dstBinding) const
    {
        static vk::DescriptorImageInfo info{};

        info = vk::DescriptorImageInfo{
            *sampler,                               // sampler
            *view,                                  // imageView
            vk::ImageLayout::eShaderReadOnlyOptimal // imageLayout
        };

        static vk::WriteDescriptorSet set{};

        set = vk::WriteDescriptorSet{
            descriptorSet,                             // dstSet
            dstBinding,                                // dstBinding
            0,                                         // dstArrayElement
            1,                                         // descriptorCount
            vk::DescriptorType::eCombinedImageSampler, // descriptorType
            &info,                                     // pImageInfo
            nullptr                                    // pBufferInfo
        };
        return set;
    }
}