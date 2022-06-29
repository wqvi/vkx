//
// Created by december on 6/21/22.
//

#pragma once

#include <renderer/core/renderer_base.hpp>

namespace vkx {
    struct ApplicationConfig {
        const char *title;
        int windowWidth;
        int windowHeight;
    };

    // A SDL window wrapper class
    // It has unique ownership over the pointer
    class TestWindow {
        // Helper class that default initializes thus making the construction of the
        // managed pointer much simpler looking
        // This is honestly syntactic constructor sugar in the source files
        struct SDL_Deleter {
            void operator()(SDL_Window *ptr) {
                SDL_DestroyWindow(ptr);
            }
        };

    public:
        // Creates a window with the information from the config
        // After the creation of the window, the projection matrix is created with the same config data
        explicit TestWindow(const ApplicationConfig &config);

        [[nodiscard]] std::pair<int, int> getSize() const noexcept;

        [[nodiscard]] int getWidth() const noexcept;

        [[nodiscard]] int getHeight() const noexcept;

    private:
        constexpr static const glm::f32 nearZ = 0.05f;
        constexpr static const glm::f32 farZ = 100.0f;

        std::unique_ptr<SDL_Window, SDL_Deleter> window;

        glm::mat4 projection = glm::mat4(1); // Initialize to identity matrix
    };

    class Application {
    public:
        Application() = delete;

        explicit Application(const ApplicationConfig &config);

        ~Application();

        void run();

    protected:
        glm::mat4 windowProjection = glm::mat4(1.0f);

    private:
        void pollEvents(SDL_Event *event);

        void pollWindowEvent(const SDL_WindowEvent &event);

        void handleResizeEvent(Sint32 width, Sint32 height);

        void handleKeyPressedEvent(const SDL_KeyboardEvent &event);

        void handleKeyReleasedEvent(const SDL_KeyboardEvent &event);

        void handleMouseMovedEvent(const SDL_MouseMotionEvent &event);

        SDL_Window *window = nullptr;

        bool isRunning = false;

        const float nearZ = 0.1f;
        const float farZ = 100.0f;

        RendererBase renderer;
    };
}