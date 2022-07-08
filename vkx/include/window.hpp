#pragma once

#include <vkx_types.hpp>
#include <scene.hpp>
#include <renderer/core/context.hpp>

namespace vkx {
    // A SDL cWindow wrapper class
    // It has unique ownership over the pointer
    class SDLWindow {
        // Helper class that default initializes thus making the construction of the
        // managed pointer much simpler looking
        struct SDL_Deleter {
            void operator()(SDL_Window *ptr) const noexcept;
        };

        friend vk::UniqueSurfaceKHR
        RendererContext::createSurface(std::shared_ptr<SDLWindow> const &window) const;
    public:
        using EventFun [[maybe_unused]] = void (*)(SDL_WindowEvent *);

        SDLWindow() = default;

        explicit SDLWindow(const char *title, int width, int height);

        explicit operator const SDL_Window *() const noexcept;

        void
        show() const noexcept;

        void
        hide() const noexcept;

        [[nodiscard]]
        std::pair<int, int>
        getSize() const noexcept;

        [[maybe_unused]]
        [[nodiscard]]
        int
        getWidth() const noexcept;

        [[maybe_unused]]
        [[nodiscard]]
        int
        getHeight() const noexcept;

        void
        pollWindowEvent(const SDL_WindowEvent &event,
                        Scene *scene);

        void
        handleResizeEvent(const SDL_WindowEvent &event,
                          Scene *scene);

        [[nodiscard]]
        std::vector<const char *>
        getExtensions() const;

        void
        waitForEvents() const;

        [[nodiscard]]
        bool
        isFramebufferResized() const noexcept;

        void
        setFramebufferResized(bool flag) noexcept;

    private:
        bool framebufferResized = false;
        std::unique_ptr<SDL_Window, SDL_Deleter> cWindow;
    };

}