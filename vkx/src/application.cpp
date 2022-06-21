//
// Created by december on 6/21/22.
//

#include <application.hpp>
#include <SDL2/SDL.h>

vkx::App::App(const vkx::AppConfig &configuration) {
    auto initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO;
    auto initCode = SDL_Init(initFlags);

    if (initCode != 0) {
        std::error_code sdlErrorCode = {initCode, std::generic_category()};
        auto sdlErrorMsg = SDL_GetError();
        throw std::system_error(sdlErrorCode, sdlErrorMsg);
    }
}
