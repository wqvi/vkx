#pragma once

#include <vkx/renderer/renderer.hpp>

namespace vkx {
class application {
private:
	bool isRunning;
	vkx::VulkanInstance instance;

public:
	SDL_Window* window;

	application();

	~application();

	void run();

	void poll();
};
}
