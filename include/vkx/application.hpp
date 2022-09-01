#pragma once

namespace vkx {
class SDLWindow {
private:
	struct SDLDeleter {
		void operator()(SDL_Window* ptr) noexcept;
	};

	std::unique_ptr<SDL_Window, SDLDeleter> window;

public:
	explicit SDLWindow(const char* title, int width, int height);

	explicit operator SDL_Window*() const noexcept;

	void show() const;

	void hide() const;

	std::pair<int, int> getSize() const;
};
} // namespace vkx