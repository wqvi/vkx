//
// Created by december on 6/21/22.
//

#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

namespace vkx {
    struct AppConfig {
        int windowWidth;
        int windowHeight;
    };

    class App {
    public:
        App() = default;

        explicit App(const AppConfig &config);

        ~App();

        void run();

    private:
        SDL_Window* window = nullptr;
        VkInstance instance = nullptr;
        VkSurfaceKHR surface = nullptr;
    };
}