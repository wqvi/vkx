#include <window.hpp>

#include <vkx_exceptions.hpp>

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

    int sdlErrorCode = SDL_ShowCursor(SDL_DISABLE);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    sdlErrorCode = SDL_SetRelativeMouseMode(SDL_TRUE);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }
}

vkx::SDLWindow::operator const SDL_Window *() const noexcept {
    return window.get();
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
    switch (event.event) {
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

vk::UniqueSurfaceKHR vkx::SDLWindow::createSurface(const vk::UniqueInstance &instance) const {
    VkSurfaceKHR surface = nullptr;
    if (SDL_Vulkan_CreateSurface(window.get(), *instance, &surface) != SDL_TRUE) {
        throw vkx::VulkanError("Failure to create VkSurfaceKHR via the SDL2 API.");
    }
    return vk::UniqueSurfaceKHR(surface, *instance);
}

std::vector<const char *> vkx::SDLWindow::getExtensions() const {
    std::uint32_t count = 0;

    if (SDL_Vulkan_GetInstanceExtensions(window.get(), &count, nullptr) != SDL_TRUE) {
        throw vkx::SDLError();
    }

    std::vector<const char *> extensions(count);

    if (SDL_Vulkan_GetInstanceExtensions(window.get(), &count, extensions.data()) != SDL_TRUE) {
        throw vkx::SDLError();
    }

#ifdef DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

void vkx::SDLWindow::waitForEvents() const {
    auto [width, height] = getSize();
    while (width == 0 || height == 0) {
        std::tie(width, height) = getSize();
        SDL_WaitEvent(nullptr);
    }
}

bool vkx::SDLWindow::isFramebufferResized() const noexcept {
    return framebufferResized;
}

void vkx::SDLWindow::setFramebufferResized(bool flag) noexcept {
    framebufferResized = flag;
}
