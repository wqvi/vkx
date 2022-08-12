#pragma once

#include <vkx/renderer/core/device.hpp>

namespace vkx {
class RendererBootstrap {
public:
	explicit RendererBootstrap(SDL_Window* window);

	[[nodiscard]]
	std::shared_ptr<vkx::Device> createDevice() const; 

private:
	vk::UniqueInstance instance{};
	vk::UniqueSurfaceKHR surface{};

	static vk::PhysicalDevice getBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);
};
} // namespace vkx
