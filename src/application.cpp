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

vkx::SDLWindow::operator SDL_Window*() const noexcept {
	return window.get();
}

void vkx::SDLWindow::show() const {
	SDL_ShowWindow(window.get());
}

void vkx::SDLWindow::hide() const {
	SDL_HideWindow(window.get());
}

std::pair<int, int> vkx::SDLWindow::getSize() const {
	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window.get(), &width, &height);

	return std::make_pair(width, height);
}