//
// Created by december on 6/21/22.
//

#include <application.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "iostream"

vkx::SDLWindow::SDLWindow(std::string_view title, std::uint32_t width, std::uint32_t height)
    : internalHandle(SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN | SDL_WINDOW_MOUSE_GRABBED)){
    if (internalHandle == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }
}

vkx::SDLWindow::~SDLWindow() {
    if (internalHandle) {
        SDL_DestroyWindow(internalHandle);
    }
}

vkx::SDLWindow::operator bool() const {
    return open;
}

void vkx::SDLWindow::show() const {
    SDL_ShowWindow(internalHandle);
}

void vkx::SDLWindow::hide() const {
    SDL_HideWindow(internalHandle);
}

bool vkx::SDLWindow::isOpen() const {
    return open;
}

void vkx::SDLWindow::pollEvents(const SDL_Event &event) {

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

    window = SDLWindow("Hello World\0", 640 ,360);
}

vkx::App::~App() {
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

void vkx::App::run() {
    window.show();
    while (window.isOpen()) {

    }
    std::cout << "Running Application!\n";
}
