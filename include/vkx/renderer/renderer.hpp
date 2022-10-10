#pragma once

#include <vkx/application.hpp>
#include <vkx/renderer/core/bootstrap.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/model.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
[[nodiscard]] vk::UniqueInstance createInstance(SDL_Window* const window);

[[nodiscard]] vk::UniqueSurfaceKHR createSurface(SDL_Window* const window, vk::Instance instance);

[[nodiscard]] vk::PhysicalDevice getBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);
} // namespace vkx
