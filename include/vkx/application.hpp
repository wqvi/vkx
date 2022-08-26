#pragma once

namespace vkx {
class Application {
public:
	Application();

	Application(const Application&) = default;

	Application(Application&&) noexcept = default;

    virtual ~Application();

	Application& operator=(const Application& other) = default;

	Application& operator=(Application&&) noexcept = default;

	virtual void init() = 0;

	virtual void destroy() = 0;
};
} // namespace vkx