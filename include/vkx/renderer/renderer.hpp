#pragma once

#include <vkx/renderer/core/bootstrap.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/application.hpp>

namespace vkx {
class Renderer {
private:
    RendererBootstrap bootstrap{};
    Device device{};
    Allocator allocator{};
    CommandSubmitter commandSubmitter{};

public:
    Renderer() = default;

    explicit Renderer(const SDLWindow& window);
};
} // namespace vkx