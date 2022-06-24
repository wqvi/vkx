//
// Created by december on 6/21/22.
//

#include <application.hpp>

vkx::Application::Application(const vkx::ApplicationConfig &config) {
    int sdlErrorCode = SDL_Init(SDL_INIT_EVERYTHING);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
    window = SDL_CreateWindow(config.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.windowWidth,
                              config.windowHeight, flags);
    if (window == nullptr) {
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

    float aspectRatio = static_cast<float>(config.windowWidth) / static_cast<float>(config.windowHeight);
    windowProjection = glm::perspective(glm::radians(75.0f), aspectRatio, nearZ, farZ);
    windowProjection[1][1] *= -1.0f;

    renderer = vkx::RendererBase{window, vkx::Profile{}};
}

vkx::Application::~Application() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void vkx::Application::run() {
    isRunning = true;
    SDL_ShowWindow(window);

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
                pollWindowEvent(event->window);
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
        }
    }
}

void vkx::Application::pollWindowEvent(const SDL_WindowEvent &event) {
    switch (event.type) {
        case SDL_WINDOWEVENT_RESIZED:
            handleResizeEvent(event.data1, event.data2);
            break;
    }
}

void vkx::Application::handleResizeEvent(Sint32 width, Sint32 height) {
    // framebufferResized = true;
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    windowProjection = glm::perspective(glm::radians(75.0f), aspectRatio, nearZ, farZ);
    windowProjection[1][1] *= -1.0f;
}

void vkx::Application::handleKeyPressedEvent(const SDL_KeyboardEvent &event) {

}

void vkx::Application::handleKeyReleasedEvent(const SDL_KeyboardEvent &event) {

}

void vkx::Application::handleMouseMovedEvent(const SDL_MouseMotionEvent &event) {

}


