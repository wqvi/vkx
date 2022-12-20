#include <vkx/window.hpp>

std::once_flag vkx::Window::initialized = {};

vkx::Window::Window(const char* name, std::int32_t width, std::int32_t height) {
	std::call_once(
	    initialized, [](auto flags) {
		    if (SDL_Init(flags) != 0) {
				throw std::runtime_error(SDL_GetError());
			}
		}, SDL_INIT_EVERYTHING);
}