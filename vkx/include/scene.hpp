//
// Created by december on 6/26/22.
//

#pragma once

#include <vkx_types.hpp>
#include <renderer/core/renderer_types.hpp>
#include <renderer/viewport.hpp>

namespace vkx {
    struct Scene {
        Scene() = default;

        virtual ~Scene() = default;

        virtual void init(const vkx::ApplicationConfig *config,
                          vkx::Application *data,
                          const vkx::RendererBase &rendererState) = 0;

        virtual void update() = 0;

        virtual void physics(float deltaTime) = 0;

        virtual void destroy() noexcept = 0;

        virtual void onKeyPress(const SDL_KeyboardEvent &event) = 0;

        virtual void onKeyRelease(const SDL_KeyboardEvent &event) = 0;

        virtual void onMouseMove(const SDL_MouseMotionEvent &event) = 0;

        virtual void onWindowResize(Sint32 width, Sint32 height) = 0;

        Viewport &getViewport();

    private:
        Viewport viewport;
    };
}
