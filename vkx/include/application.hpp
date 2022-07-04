//
// Created by december on 6/21/22.
//

#pragma once

#include <renderer/core/renderer_base.hpp>
#include <scene.hpp>

namespace vkx {
    struct ApplicationConfig {
        const char *title = "VKX Application";
        int windowWidth = 640;
        int windowHeight = 360;
        glm::f32 fov = 70.0f;
    };

    class Application {
    public:
        Application() = delete;

        explicit Application(const ApplicationConfig &config);

        ~Application();

        void run();

        void setScene(Scene *newScene);

    private:
        void pollEvents(SDL_Event *event);

        void handleKeyPressedEvent(const SDL_KeyboardEvent &event);

        void handleKeyReleasedEvent(const SDL_KeyboardEvent &event);

        void handleMouseMovedEvent(const SDL_MouseMotionEvent &event);

        ApplicationConfig config;

        SDLWindow window;

        bool isRunning = false;

        RendererBase renderer;

        std::unique_ptr<Scene> scene = nullptr;
    };
}