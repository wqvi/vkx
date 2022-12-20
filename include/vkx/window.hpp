#pragma once

namespace vkx {
class Window {
private:
	static std::once_flag initialized;
	SDL_Window* windowHandle = nullptr;

public:
	Window() = default;

	explicit Window(const char* name, std::int32_t width, std::int32_t height);
};
} // namespace vkx