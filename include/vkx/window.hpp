#pragma once

namespace vkx {
class Window {
public:
	Window() = default;

	explicit Window(const char* name, std::int32_t width, std::int32_t height);
};
} // namespace vkx