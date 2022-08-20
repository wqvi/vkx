#include <vkx/sdl.hpp>

void vkx::SDL::Deleter::operator()(SDL_Window* ptr) noexcept {
	if (ptr != nullptr) {
		SDL_DestroyWindow(ptr);
		SDL_Quit();
	}
}

vkx::SDL::SDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::string errorMessage = std::string(__FILE__) + std::string(": Failure to initialize SDL2: ") + std::string(SDL_GetError());
		throw std::runtime_error(errorMessage);
	}
}