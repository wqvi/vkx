#pragma once

namespace vkx {
class application {
private:
	bool isRunning;

public:
	SDL_Window* window;

	application();

	~application();

	void run();

	void poll();
};
}
