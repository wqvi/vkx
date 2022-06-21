//
// Created by december on 6/21/22.
//

#include <application.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

vkx::SDLWindow::SDLWindow(std::string_view title, std::uint32_t width, std::uint32_t height)
    : internalHandle(SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN | SDL_WINDOW_MOUSE_GRABBED)){

}

vkx::App::App(const vkx::AppConfig &configuration) {
    auto initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO;
    auto sdlInitCode = SDL_Init(initFlags);

    if (sdlInitCode < 0) {
        throw std::system_error(std::error_code(sdlInitCode, std::generic_category()),SDL_GetError());
    }

    sdlInitCode = SDL_Vulkan_LoadLibrary(nullptr);

    if (sdlInitCode != 0) {
        throw std::system_error(std::error_code(sdlInitCode, std::generic_category()),SDL_GetError());
    }


}
