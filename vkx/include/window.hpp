#pragma once

#include <util/observer.hpp>
#include <SDL2/SDL.h>

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
}