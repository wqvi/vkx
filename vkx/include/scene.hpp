//
// Created by december on 6/26/22.
//

#pragma once

#include <vkx_types.hpp>

namespace vkx {
    struct Scene {
        Scene() = default;

        virtual ~Scene() = default;

        virtual void init(const vkx::ApplicationConfig *config, const vkx::Application *data) = 0;

        virtual void update() = 0;

        virtual void physics(float deltaTime) = 0;

        virtual void destroy() noexcept = 0;

        virtual void onKeyPress() = 0;

        virtual void onKeyRelease() = 0;

        virtual void onMouseMove() = 0;

        virtual void onWindowResize() = 0;
    };
}
