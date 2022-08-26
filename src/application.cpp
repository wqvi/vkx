#include <vkx/application.hpp>

void vkx::SDLWindow::SDLDeleter::operator()(SDL_Window* ptr) noexcept {
	if (ptr) {
		SDL_DestroyWindow(ptr);
	}
}

vkx::SDLWindow::SDLWindow(const char* title, int width, int height)
    : window(std::unique_ptr<SDL_Window, vkx::SDLWindow::SDLDeleter>(SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN))) {
	if (window == nullptr) {
		throw std::runtime_error(SDL_GetError());
	}
}

vkx::Application::Application()
    : state(SDLInit()), window("vkx", 640, 360) {
        //, renderer(static_cast<SDL_Window*>(window)) {
	if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
		throw std::runtime_error(SDL_GetError());
	}
}

vkx::Application::~Application() {
	if (state == 0) {
		SDL_Quit();
	}
}

int vkx::Application::SDLInit() {
	int state = SDL_Init(SDL_INIT_EVERYTHING);

	if (state != 0) {
		throw std::runtime_error(SDL_GetError());
	}

	return state;
}