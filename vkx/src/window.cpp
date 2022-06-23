#include <window.hpp>

#include <SDL2/SDL_vulkan.h>

namespace vkx {
    Window::Window(const char *title, int width, int height) {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            throw std::runtime_error(SDL_GetError());
        }

        auto flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;

        internalHandle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          static_cast<int>(width), static_cast<int>(height),
                                          flags);

        SDL_ShowCursor(SDL_DISABLE);

        SDL_SetRelativeMouseMode(SDL_TRUE);

        if (internalHandle == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }
    }

    Window::~Window() {
        SDL_DestroyWindow(internalHandle);
        SDL_Quit();
    }

    void Window::show() const {
        SDL_ShowWindow(internalHandle);
    }

    void Window::hide() const {
        SDL_HideWindow(internalHandle);
    }

    vk::UniqueSurfaceKHR Window::createSurface(vk::UniqueInstance const &instance) const {
        VkSurfaceKHR cSurface = nullptr;
        if (SDL_Vulkan_CreateSurface(internalHandle, *instance, &cSurface) == SDL_FALSE) {
            throw std::runtime_error(SDL_GetError());
        }
        return vk::UniqueSurfaceKHR(cSurface, *instance);
    }

    std::pair<std::uint32_t, std::uint32_t> Window::getSize() const {
        int width = 0;
        int height = 0;
        SDL_Vulkan_GetDrawableSize(internalHandle, &width, &height);
        return {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    }
}