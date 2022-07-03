//
// Created by december on 6/21/22.
//

#include <application.hpp>

vkx::Application::Application(const vkx::ApplicationConfig &config)
    : config(config) {
    int sdlErrorCode = SDL_Init(SDL_INIT_EVERYTHING);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    window = vkx::SDLWindow(config.title, config.windowWidth, config.windowHeight);

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
    if (scene != nullptr) scene->destroy();
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
        scene->update();
        scene->physics(deltaTime);

        // Query next image

        // Submit draw commands

        // Present queues

        // Poll Events
        pollEvents(&event);

        // Update last time
        lastTime = currentTime;
    }
}

void vkx::Application::setScene(vkx::Scene *newScene) {
    if (newScene == nullptr) throw std::invalid_argument("New scene can't be a nullptr.");
    if (scene != nullptr) scene->destroy();
    scene.reset(newScene);
    scene->init(&config, this, renderer);
}

void vkx::Application::pollEvents(SDL_Event *event) {
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_WINDOWEVENT:
                window.pollWindowEvent(event->window, scene.get());
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
    scene->onKeyPress();
}

void vkx::Application::handleKeyReleasedEvent(const SDL_KeyboardEvent &event) {
    scene->onKeyRelease();
}

void vkx::Application::handleMouseMovedEvent(const SDL_MouseMotionEvent &event) {
    scene->onMouseMove();
}
