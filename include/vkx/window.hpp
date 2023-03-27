#pragma once

namespace vkx {
class Window {
private:
	static std::once_flag initialized;
	SDL_Window* windowHandle = nullptr;

public:
	Window() = default;

	explicit Window(const char* name, std::int32_t width, std::int32_t height);

	Window(const Window& other) = delete;

	Window(Window&& other) noexcept;

	~Window();

	Window& operator=(const Window& other) = delete;

	Window& operator=(Window&& other) noexcept;

	explicit operator SDL_Window*() const;

	std::pair<std::int32_t, std::int32_t> getDimensions() const;

	void waitForUpdate() const;

	void show() const;

	void hide() const;

	void createPopup();
};
} // namespace vkx