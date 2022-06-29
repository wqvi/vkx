//
// Created by december on 6/26/22.
//

#pragma once

namespace vkx {
    class Scene {
    public:
        Scene() = default;

        virtual ~Scene() = default;

        virtual void init() = 0;

        virtual void run() = 0;

        virtual void destroy() noexcept = 0;
    };
}
