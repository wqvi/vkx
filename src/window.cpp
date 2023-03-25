#include <vkx/window.hpp>

std::once_flag vkx::Window::initialized = {};

vkx::Window::Window(const char* name, std::int32_t width, std::int32_t height) {
	std::call_once(
	    initialized, [](auto flags) {
		    if (SDL_Init(flags) != 0) {
				throw std::runtime_error(SDL_GetError());
			}

		    if (std::atexit([]() {
				SDL_Quit();
			}) != 0) {
				throw std::runtime_error("Failed to register SDL_Quit at exit.");
		    }

			if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
				throw std::runtime_error(SDL_GetError());
			}
		}, SDL_INIT_EVERYTHING);

	windowHandle = SDL_CreateWindow(name, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
	if (windowHandle == nullptr) {
		throw std::runtime_error(SDL_GetError());
	}
}

vkx::Window::Window(Window&& other) noexcept 
	: windowHandle(std::move(other.windowHandle)) {
	other.windowHandle = nullptr;
}

vkx::Window::~Window() {
	if (windowHandle) {
		SDL_DestroyWindow(windowHandle);
	}
}

vkx::Window& vkx::Window::operator=(Window&& other) noexcept {
	windowHandle = std::move(other.windowHandle);
	other.windowHandle = nullptr;
	return *this;
}

vkx::Window::operator SDL_Window*() const {
	return windowHandle;
}

std::pair<std::int32_t, std::int32_t> vkx::Window::getDimensions() const {
	int width = 0;
	int height = 0;
	SDL_GetWindowSizeInPixels(windowHandle, &width, &height);
	return std::make_pair(width, height);
}

void vkx::Window::waitForUpdate() const {
	auto [width, height] = getDimensions();
	while (width == 0 || height == 0) {
		std::tie(width, height) = getDimensions();
		SDL_WaitEvent(nullptr);
	}
}

void vkx::Window::show() const {
	SDL_ShowWindow(windowHandle);
}

void vkx::Window::hide() const {
	SDL_HideWindow(windowHandle);
}
