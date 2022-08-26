#pragma once

#include <vkx/renderer/core/bootstrap.hpp>

namespace vkx {
class Renderer {
private:
    const RendererBootstrap bootstrap;
public:
    explicit Renderer(const SDL_Window* window);
};

class SDLWindow {
private:
    struct SDLDeleter {
        void operator()(SDL_Window* ptr) noexcept;
    };

    std::unique_ptr<SDL_Window, SDLDeleter> window;

public:
    explicit SDLWindow(const char* title, int width, int height);

    explicit operator SDL_Window*() const noexcept;
};

class Application {
private:
    int state = 1;

    SDLWindow window;

    // Renderer renderer;

public:
	Application();

	Application(const Application&) = delete;

	Application(Application&&) noexcept = default;

    virtual ~Application();

	Application& operator=(const Application& other) = delete;

	Application& operator=(Application&&) noexcept = default;

	virtual void init() = 0;

	virtual void destroy() = 0;

private:
    static int SDLInit();
};
} // namespace vkx