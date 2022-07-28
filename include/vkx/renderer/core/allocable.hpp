#pragma once

namespace vkx {
template <class T> class Allocable {
public:
  Allocable() = default;

  explicit operator T const &() const { return *obj; }

  explicit operator vk::DeviceMemory const &() const { return *memory; }

  vk::UniqueHandle<T, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> obj;
  vk::UniqueDeviceMemory memory;
};
} // namespace vkx