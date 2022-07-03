#pragma once

#include <util/observer.hpp>
#include <SDL2/SDL.h>
#include <scene.hpp>

namespace vkx {
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

        explicit operator const SDL_Window *() const noexcept;

        void show() const noexcept;

        void hide() const noexcept;

        [[nodiscard]]
        std::pair<int, int> getSize() const noexcept;

        [[nodiscard]]
        int getWidth() const noexcept;

        [[nodiscard]]
        int getHeight() const noexcept;

        void pollWindowEvent(const SDL_WindowEvent &event, Scene *scene);

        void handleResizeEvent(const SDL_WindowEvent &event, Scene *scene);

        [[nodiscard]]
        vk::UniqueSurfaceKHR createSurface(const vk::UniqueInstance &instance) const;

        [[nodiscard]]
        std::vector<const char *> getExtensions() const;

        void waitForEvents() const;

        [[nodiscard]]
        bool isFramebufferResized() const noexcept;

        void setFramebufferResized(bool flag) noexcept;

    private:
        bool framebufferResized = false;
        std::unique_ptr<SDL_Window, SDL_Deleter> window;
    };

}