#pragma once

#include <renderer/core/allocable.hpp>
#include <renderer/core/device.hpp>

namespace vkx
{
  class Image : public Allocable<vk::Image>
  {
  public:
    Image() = default;

    explicit Image(std::string const &file, Device const &device);

    explicit Image(std::byte const *data, std::size_t size, Device const &device);

    explicit Image(std::vector<std::byte> &data, Device const &device);
  };
}