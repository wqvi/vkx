#include <vkx/application.hpp>

namespace vkx {
application::application() {
#ifdef DEBUG
	SDL_Log("Hello!");
#endif

	if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC) != 0) {
		throw std::runtime_error(SDL_GetError());
	}

	window = SDL_CreateWindow("VKX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

	if (window == nullptr) {
		throw std::runtime_error(SDL_GetError());
	}
}

application::~application() {
#ifdef DEBUG
	SDL_Log("Good bye!");
#endif
	if (window) {
		SDL_DestroyWindow(window);
	}

	SDL_Quit();
}

void application::run() {
	isRunning = true;

	std::uint32_t currentFrame = 0;
	bool framebufferResized = false;

	SDL_ShowWindow(window);
	while (isRunning) {
		poll();
	}

	// cleanup?
}

void application::poll() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		const auto eventType = event.type;

		switch (eventType) {
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_WINDOWEVENT:
			break;
		case SDL_KEYDOWN:
			break;
		case SDL_KEYUP:
			break;
		case SDL_MOUSEMOTION:
			break;
		default:
			break;
		}
	}
}
}
