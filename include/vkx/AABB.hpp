//
// Created by december on 6/29/22.
//

#pragma once

namespace vkx {
    class AABB {
    public:
        AABB() = default;

        AABB(const glm::vec3 &position, const glm::vec3 &size);

        bool intersects(const AABB &other) const noexcept;

    private:
        glm::vec3 position;
        glm::vec3 size;
    };
}