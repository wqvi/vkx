//
// Created by december on 7/5/22.
//

#pragma once

#include <node.hpp>

namespace vkx {
    class Viewport : public Node {
    public:
        Viewport() = default;

        explicit Viewport(std::uint32_t fov, Sint32 width, Sint32 height);

        explicit operator const glm::mat4 &() const;

        [[nodiscard]]
        std::uint32_t getFOV() const noexcept;

        void setFOV(std::uint32_t newFOV) noexcept;

        void setSize(Sint32 newWidth, Sint32 newHeight);

    private:
        std::uint32_t fov = 70;
        glm::mat4 projection = glm::mat4{1};
        Sint32 width = 640;
        Sint32 height = 360;
    };
}
