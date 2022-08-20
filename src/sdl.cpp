#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <vkx/sdl.hpp>

void vkx::SDL::Deleter::operator()(SDL_Window *ptr) noexcept {
    if (ptr != nullptr) {
        SDL_DestroyWindow(ptr);
        SDL_Quit();
    }
}