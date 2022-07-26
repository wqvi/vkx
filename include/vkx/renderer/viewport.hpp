//
// Created by december on 7/5/22.
//

#pragma once

#include <vkx/node.hpp>

namespace vkx {
    class Viewport : public Node {
    public:
        Viewport() = default;

        explicit operator const glm::mat4 &() const;

        [[maybe_unused]]
        [[nodiscard]]
        std::uint32_t getFOV() const noexcept;

        [[maybe_unused]]
        void setFOV(std::uint32_t _fov) noexcept;

        [[maybe_unused]]
        void setSize(Sint32 _width, Sint32 _height);

    private:
        std::uint32_t fov = 70;
        glm::mat4 projection = glm::mat4{1};
        // In the application the width and height values will be overridden with the appropriate values
        Sint32 width = SDL_MAX_SINT32;
        Sint32 height = SDL_MAX_SINT32;
    };
}
