//
// Created by december on 6/21/22.
//

#include <application.hpp>

const char *vkx::SDLError::what() const noexcept {
    return SDL_GetError();
}

vkx::VulkanError::VulkanError(const char *message)
        : message(message) {}

vkx::VulkanError::VulkanError(vk::Result result)
        : message(vk::to_string(result)) {}

const char *vkx::VulkanError::what() const noexcept {
    return message.c_str();
}

vkx::SDLWindow::SDLWindow(const ApplicationConfig &config) {
    SDL_Window *sdlWindow = SDL_CreateWindow(config.title,
                                             SDL_WINDOWPOS_UNDEFINED,
                                             SDL_WINDOWPOS_UNDEFINED,
                                             config.windowWidth,
                                             config.windowHeight,
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

void vkx::SDLWindow::pollWindowEvent(const SDL_WindowEvent &event) {
    switch (event.type) {
        case SDL_WINDOWEVENT_RESIZED:
            handleResizeEvent(event);
            break;
        default:
            return;
    }
}

void vkx::SDLWindow::handleResizeEvent(const SDL_WindowEvent &event) {
    framebufferResized = true;
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

vkx::Application::Application(const vkx::ApplicationConfig &config) {
    int sdlErrorCode = SDL_Init(SDL_INIT_EVERYTHING);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    window = SDLWindow(config);

    sdlErrorCode = SDL_ShowCursor(SDL_DISABLE);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    sdlErrorCode = SDL_SetRelativeMouseMode(SDL_TRUE);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

//    renderer = vkx::RendererBase{window, vkx::Profile{}};
}

vkx::Application::~Application() {
    SDL_Quit();
}

void vkx::Application::run() {
    isRunning = true;

    window.show();

    // Declared outside the loop, so it is only initialized once on our side
    SDL_Event event{};

    std::chrono::system_clock::time_point lastTime = std::chrono::system_clock::now();
    while (isRunning) {
        // Prep delta time
        auto currentTime = std::chrono::system_clock::now();
        auto deltaTime = std::chrono::duration<float, std::chrono::milliseconds::period>(
                currentTime - lastTime).count();

        // Populate uniforms

        // Query next image

        // Submit draw commands

        // Present queues

        // Poll Events
        pollEvents(&event);

        // Update last time
        lastTime = currentTime;
    }
}

void vkx::Application::pollEvents(SDL_Event *event) {
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_WINDOWEVENT:
                window.pollWindowEvent(event->window);
                break;
            case SDL_KEYDOWN:
                handleKeyPressedEvent(event->key);
                break;
            case SDL_KEYUP:
                handleKeyReleasedEvent(event->key);
                break;
            case SDL_MOUSEMOTION:
                handleMouseMovedEvent(event->motion);
                break;
            default:
                return;
        }
    }
}

void vkx::Application::handleKeyPressedEvent(const SDL_KeyboardEvent &event) {

}

void vkx::Application::handleKeyReleasedEvent(const SDL_KeyboardEvent &event) {

}

void vkx::Application::handleMouseMovedEvent(const SDL_MouseMotionEvent &event) {

}
