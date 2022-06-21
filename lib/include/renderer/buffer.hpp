#pragma once

#include <renderer/core/allocable.hpp>
#include <renderer/core/device.hpp>

namespace vkx
{
  class Buffer : public Allocable<vk::Buffer>
  {
  public:
    Buffer() = default;

    Buffer(std::vector<Vertex> const &vertices, Device const &device);

    Buffer(std::vector<std::uint32_t> const &indices, Device const &device);

    Buffer(std::size_t size, vk::BufferUsageFlags const &usage, Device const &device);

  private:
    template <class T>
    explicit Buffer(T const *data, std::size_t size, vk::BufferUsageFlags const &usage, Device const &device);
  };
}