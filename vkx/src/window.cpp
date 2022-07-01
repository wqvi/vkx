#include <window.hpp>

#include <SDL2/SDL_vulkan.h>

#include <vkx_exceptions.hpp>

namespace vkx {
    Window::Window(const char *title, int width, int height) {
        int sdlErrorCode = SDL_Init(SDL_INIT_EVERYTHING);
        if (sdlErrorCode < 0) {
            throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
        }

        Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;

        internalHandle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

        if (internalHandle == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }

        sdlErrorCode = SDL_ShowCursor(SDL_DISABLE);
        if (sdlErrorCode < 0) {
            throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
        }

        sdlErrorCode = SDL_SetRelativeMouseMode(SDL_TRUE);
        if (sdlErrorCode < 0) {
            throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
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

vkx::SDLWindow::SDLWindow(const char *title, int width, int height) {
    SDL_Window *sdlWindow = SDL_CreateWindow(title,
                                             SDL_WINDOWPOS_UNDEFINED,
                                             SDL_WINDOWPOS_UNDEFINED,
                                             width,
                                             height,
                                             SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (sdlWindow == nullptr) {
        throw vkx::SDLError();
    }

    window = std::unique_ptr<SDL_Window, SDL_Deleter>(sdlWindow);
}

void vkx::SDLWindow::show() const noexcept {
    SDL_ShowWindow(window.get());
}

void vkx::SDLWindow::hide() const noexcept {
    SDL_HideWindow(window.get());
}

std::pair<int, int> vkx::SDLWindow::getSize() const noexcept {
    int width;
    int height;

    SDL_GetWindowSize(window.get(), &width, &height);

    return std::make_pair(width, height);
}

int vkx::SDLWindow::getWidth() const noexcept {
    int width;

    SDL_GetWindowSize(window.get(), &width, nullptr);

    return width;
}

int vkx::SDLWindow::getHeight() const noexcept {
    int height;

    SDL_GetWindowSize(window.get(), nullptr, &height);

    return height;
}

void vkx::SDLWindow::pollWindowEvent(const SDL_WindowEvent &event, vkx::Scene *scene) {
    switch (event.type) {
        case SDL_WINDOWEVENT_RESIZED:
            handleResizeEvent(event, scene);
            break;
        case SDL_WINDOWEVENT_ENTER:
            // TODO Stop performance mode
            break;
        case SDL_WINDOWEVENT_LEAVE:
            // TODO Start performance mode
            break;
        default:
            return;
    }
}

void vkx::SDLWindow::handleResizeEvent(const SDL_WindowEvent &event, vkx::Scene *scene) {
    framebufferResized = true;
    // data1 x data2 is width x height
    scene->onWindowResize(event.data1, event.data2);
}

bool vkx::SDLWindow::isResized() const noexcept {
    return framebufferResized;
}

vk::UniqueSurfaceKHR vkx::SDLWindow::createSurface(const vk::UniqueInstance &instance) const {
    VkSurfaceKHR surface = nullptr;
    if (SDL_Vulkan_CreateSurface(window.get(), *instance, &surface) != SDL_TRUE) {
        throw vkx::VulkanError("Failure to create VkSurfaceKHR via the SDL2 API.");
    }
    return vk::UniqueSurfaceKHR(surface, *instance);
}