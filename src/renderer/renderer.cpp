#include "vkx/renderer/core/bootstrap.hpp"
#include <vkx/renderer/renderer.hpp>

vkx::Renderer::Renderer(const SDLWindow& window) 
    : bootstrap(static_cast<SDL_Window*>(window)) {}