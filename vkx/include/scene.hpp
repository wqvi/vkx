//
// Created by december on 6/26/22.
//

#pragma once

#include <vkx_types.hpp>
#include <renderer/core/renderer_types.hpp>

namespace vkx {
    struct Scene {
        Scene() = default;

        virtual ~Scene() = default;

        virtual void init(const vkx::ApplicationConfig *config,
                          const vkx::Application *data,
                          const vkx::RendererBase &rendererState) = 0;

        virtual void update() {};

        virtual void physics(float deltaTime) {};

        virtual void destroy() noexcept {};

        virtual void onKeyPress() {};

        virtual void onKeyRelease() {};

        virtual void onMouseMove() {};

        virtual void onWindowResize(Sint32 width, Sint32 height) {};
    };
}
