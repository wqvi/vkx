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
        TestWindow() = default;

        // Creates a window with the information from the config
        explicit TestWindow(const ApplicationConfig &config);

        void show() const noexcept;

        void hide() const noexcept;

        [[nodiscard]] std::pair<int, int> getSize() const noexcept;

        [[nodiscard]] int getWidth() const noexcept;

        [[nodiscard]] int getHeight() const noexcept;

        void pollWindowEvent(const SDL_WindowEvent &event);

        void handleResizeEvent(const SDL_WindowEvent &event);

        [[nodiscard]] bool isResized() const noexcept;

        [[nodiscard]] vk::UniqueSurfaceKHR createSurface(const vk::UniqueInstance &instance) const;

    private:
        bool framebufferResized = false;

        std::unique_ptr<SDL_Window, SDL_Deleter> window;

        typedef void (*SDLEventFunction)(SDL_WindowEvent *);
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

        TestWindow window;

        bool isRunning = false;

        RendererBase renderer;
    };
}