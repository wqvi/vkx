#pragma once

#include "vkx/renderer/core/allocable.hpp"
#include "vkx/renderer/core/device.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
class Image : public Allocable<vk::Image> {
public:
  std::shared_ptr<Allocation<vk::Image>> resource;

	Image() = default;

	explicit Image(const std::string& file, const Device& device, const std::shared_ptr<Allocator>& allocator);
};
} // namespace vkx