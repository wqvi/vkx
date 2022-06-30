#pragma once

#include <util/observer.hpp>
#include <SDL2/SDL.h>
#include <scene.hpp>

namespace vkx {
    class Window {
    public:
        Window() = delete;

        Window(const char *title, int width, int height);

        ~Window();

        [[maybe_unused]]
        void show() const;

        [[maybe_unused]]
        void hide() const;

        [[nodiscard]]
        vk::UniqueSurfaceKHR createSurface(vk::UniqueInstance const &instance) const;

        [[nodiscard]]
        std::pair<std::uint32_t, std::uint32_t> getSize() const;

        SDL_Window *internalHandle;
    };

    // A SDL window wrapper class
    // It has unique ownership over the pointer
    class SDLWindow {
        // Helper class that default initializes thus making the construction of the
        // managed pointer much simpler looking
        struct SDL_Deleter {
            void operator()(SDL_Window *ptr) const {
                SDL_DestroyWindow(ptr);
            }
        };

    public:
        using EventFun = void (*)(SDL_WindowEvent *);

        SDLWindow() = default;

        explicit SDLWindow(const char *title, int width, int height);

        void show() const noexcept;

        void hide() const noexcept;

        [[nodiscard]] std::pair<int, int> getSize() const noexcept;

        [[nodiscard]] int getWidth() const noexcept;

        [[nodiscard]] int getHeight() const noexcept;

        void pollWindowEvent(const SDL_WindowEvent &event, Scene *scene);

        void handleResizeEvent(const SDL_WindowEvent &event, Scene *scene);

        [[nodiscard]] bool isResized() const noexcept;

        [[nodiscard]] vk::UniqueSurfaceKHR createSurface(const vk::UniqueInstance &instance) const;

    private:
        bool framebufferResized = false;

        std::unique_ptr<SDL_Window, SDL_Deleter> window;
    };

}