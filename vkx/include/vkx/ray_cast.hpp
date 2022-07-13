//
// Created by december on 7/11/22.
//

#pragma once

namespace vkx {
    class RayCast {
    public:
        RayCast() = default;

    private:
        glm::vec3 lookingAt = glm::vec3{0.0f, 0.0f, 0.0f};
        glm::vec3 eyePosition = glm::vec3{0.0f, 0.0f, 0.0f};
    };
}