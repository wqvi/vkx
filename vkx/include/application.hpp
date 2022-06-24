//
// Created by december on 6/21/22.
//

#pragma once

namespace vkx {
    struct ApplicationConfig {
        const char *title;
        int windowWidth;
        int windowHeight;
    };

    class Application {
    public:
        Application() = delete;

        explicit Application(const ApplicationConfig &config);

        ~Application();

        void run();

    private:
        void pollEvents(SDL_Event *event);

        void pollWindowEvent(const SDL_WindowEvent &event);

        void handleResizeEvent(Sint32 width, Sint32 height);

        void handleKeyPressedEvent(const SDL_KeyboardEvent &event);

        void handleKeyReleasedEvent(const SDL_KeyboardEvent &event);

        void handleMouseMovedEvent(const SDL_MouseMotionEvent &event);

        SDL_Window *window = nullptr;

        bool isRunning = false;

        glm::mat4 windowProjection = glm::mat4(1.0f);
        const float nearZ = 0.1f;
        const float farZ = 100.0f;
    };
}