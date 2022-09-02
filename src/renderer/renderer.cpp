#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::Renderer::Renderer(const SDLWindow& window)
    : bootstrap(static_cast<SDL_Window*>(window)),
      device(bootstrap.createDevice()),
      allocator(device.createAllocator()),
      commandSubmitter(device.createCommandSubmitter()) {
	const vkx::SwapchainInfo swapchainInfo{device};
    const auto surfaceFormat = swapchainInfo.chooseSurfaceFormat().format;
}