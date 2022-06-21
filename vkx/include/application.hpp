//
// Created by december on 6/21/22.
//

#pragma once

#include <SDL2/SDL_video.h>

namespace vkx {
    struct AppConfig {
        std::uint32_t windowWidth;
        std::uint32_t windowHeight;
    };

    class SDLWindow {
    public:
        SDLWindow(std::string_view title, std::uint32_t width, std::uint32_t height);

        ~SDLWindow();

    private:
        SDL_Window* internalHandle;
    };

    class App {
    public:
        App() = default;

        App(const AppConfig &configuration);

        ~App() = default;

        void run();
    };
}