//
// Created by december on 6/21/22.
//

#pragma once

#include <SDL2/SDL.h>

namespace vkx {
    struct AppConfig {
        std::uint32_t windowWidth;
        std::uint32_t windowHeight;
    };

    class SDLWindow {
    public:
        SDLWindow() = default;

        SDLWindow(std::string_view title, std::uint32_t width, std::uint32_t height);

        ~SDLWindow();

        operator bool() const;

        void show() const;

        void hide() const;

        [[nodiscard]] bool isOpen() const;

        void pollEvents(const SDL_Event &event);

    private:
        SDL_Window* internalHandle = nullptr;
        bool open = false;
    };

    class App {
        friend void SDLWindow::pollEvents(const SDL_Event &event);

    public:
        App() = default;

        App(const AppConfig &configuration);

        ~App();

        void run();

    private:
        SDLWindow window;
    };
}