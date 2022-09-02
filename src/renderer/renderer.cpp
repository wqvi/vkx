#include "vkx/renderer/core/allocator.hpp"
#include "vkx/renderer/core/bootstrap.hpp"
#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/renderer.hpp>

vkx::Renderer::Renderer(const SDLWindow& window) 
    : bootstrap(static_cast<SDL_Window*>(window)),
    device(bootstrap.createDevice()),
    allocator(device.createAllocator()),
    commandSubmitter(device.createCommandSubmitter()) {}