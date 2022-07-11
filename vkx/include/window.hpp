#pragma once

#include <vkx_types.hpp>
#include <scene.hpp>
#include <renderer/core/context.hpp>

namespace vkx {
    // A SDL c Window wrapper class
    // It has unique ownership over the pointer
    class SDLWindow {
        // Helper class that default initializes thus making the construction of the
        // managed pointer much simpler looking
        struct SDLDeleter {
            void operator()(SDL_Window *ptr) const noexcept;
        };

        // Friend function to create a surface, this function only needs to access the SDL_Window* variable
        friend vk::UniqueSurfaceKHR
        RendererContext::createSurface(std::shared_ptr<SDLWindow> const &window) const;

    public:
        // Construct a base window, it uses C types as the C api it interfaces with use weakly typed parameters
        explicit SDLWindow(char const *title, int width, int height);

        // Makes the window visible
        void
        show() const noexcept;

        // Makes the window invisible
        // Most likely not being used
        [[maybe_unused]]
        void
        hide() const noexcept;

        // Gets the size of the window
        // Once again weakly sized types as the C api uses weakly sized types
        [[nodiscard]]
        std::pair<int, int>
        getSize() const noexcept;

        // Width of window
        [[maybe_unused]] [[maybe_unused]]
        [[nodiscard]]
        int
        getWidth() const noexcept;

        // Height of window
        [[maybe_unused]]
        [[nodiscard]]
        int
        getHeight() const noexcept;

        // Called in the vkx::Application::pollEvents function
        // Polls window specific events, such as resizing of the window
        void
        pollWindowEvent(const SDL_WindowEvent &event,
                        Scene *scene);

        // The window resize handler. Resizes main viewport automatically
        void
        handleResizeEvent(const SDL_WindowEvent &event,
                          Scene *scene);

        // Gets the vulkan extensions for the SDL surface
        [[nodiscard]]
        std::vector<char const *>
        getExtensions() const;

        // Used when waiting for the swapchain to be recreated upon resize
        void
        waitForEvents() const;

        // Simple getter for if the main framebuffer has been resized
        [[nodiscard]]
        bool
        isFramebufferResized() const noexcept;

        // Simple setter for if the main framebuffer has been resized
        void
        setFramebufferResized(bool flag) noexcept;

        // Sets if the cursor is locked to the window and invisible
        static void
        setCursorRelative(bool relative);

    private:
        bool framebufferResized = false;
        std::unique_ptr<SDL_Window, SDLDeleter> cWindow;
    };

}