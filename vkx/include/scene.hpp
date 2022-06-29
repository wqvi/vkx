//
// Created by december on 6/26/22.
//

#pragma once

namespace vkx {
    template <class T>
    class Scene {
    public:
        Scene() = default;

        virtual ~Scene() = default;

        virtual void init(const T &data) = 0;

        virtual void update() = 0;

        virtual void physics(float deltaTime) = 0;

        virtual void destroy() noexcept = 0;
    };
}
