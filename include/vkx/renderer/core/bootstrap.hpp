#pragma once

#include <vkx/renderer/core/device.hpp>

namespace vkx {
class RendererBootstrap {
public:
	explicit RendererBootstrap(SDL_Window* window);

	RendererBootstrap(const RendererBootstrap&) = delete;

	RendererBootstrap(RendererBootstrap&&) noexcept = default;

	RendererBootstrap& operator=(const RendererBootstrap&) = delete;

	RendererBootstrap& operator=(RendererBootstrap&&) noexcept = default;

	[[nodiscard]]
	vkx::Device createDevice() const; 

private:
	vk::UniqueInstance instance{};
	vk::UniqueSurfaceKHR surface{};

	static vk::PhysicalDevice getBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);
};
} // namespace vkx
