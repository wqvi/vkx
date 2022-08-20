#pragma once

namespace vkx {
class SDL {
	struct Deleter {
		void operator()(SDL_Window* ptr) noexcept;
	};

    std::unique_ptr<SDL_Window, Deleter> window;

public:
	SDL();

    operator SDL_Window*() const;

    [[nodiscard]]
    vk::UniqueSurfaceKHR createSurface(vk::Instance instance) const;
};
} // namespace vkx