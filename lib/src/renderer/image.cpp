#include <renderer/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace vkx
{
  Image::Image(std::string const &file, Device const &device)
  {
    int texWidth;
    int texHeight;
    int texChannels;
    auto *pixels = stbi_load(file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels)
    {
      throw std::runtime_error("Failed to load texture image.");
    }

    auto imageSize = static_cast<vk::DeviceSize>(texWidth * texHeight * STBI_rgb_alpha);

    auto stagingBuffer = device.createBufferUnique(imageSize, vk::BufferUsageFlagBits::eTransferSrc);
    auto stagingBufferMemory = device.allocateMemoryUnique(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    auto *mappedMemory = static_cast<stbi_uc *>(device->mapMemory(*stagingBufferMemory, 0, imageSize, {}));
    std::memcpy(mappedMemory, pixels, imageSize);
    device->unmapMemory(*stagingBufferMemory);

    obj = device.createImageUnique(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
    memory = device.allocateMemoryUnique(obj, vk::MemoryPropertyFlagBits::eDeviceLocal);

    device.transitionImageLayout(*obj, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    device.copyBufferToImage(*stagingBuffer, *obj, texWidth, texHeight);
    device.transitionImageLayout(*obj, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    stbi_image_free(pixels);
  }
}