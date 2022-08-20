#pragma once

namespace vkx {
class SDL {
	struct Deleter {
		void operator()(SDL_Window* ptr) noexcept;
	};

public:
	SDL();
};
} // namespace vkx