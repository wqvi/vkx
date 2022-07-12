//
// Created by december on 7/11/22.
//

#pragma once

namespace vkx {
    class RayCast {
    public:
        RayCast() = default;

    private:
        glm::vec3 lookingAt;
        glm::vec3 eyePosition;
    };
}