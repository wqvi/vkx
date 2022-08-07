#pragma once

#include "vkx/renderer/core/allocable.hpp"
#include "vkx/renderer/core/device.hpp"

namespace vkx
{
  class Image : public Allocable<vk::Image>
  {
  public:
    Image() = default;

    explicit Image(std::string const &file, Device const &device);
  };
}