#pragma once

#include <vkx/aabb.hpp>

namespace vkx {
using RaycastPredicate = std::function<bool(const glm::ivec3&)>;

struct RaycastResult {
    bool success = false;
    glm::ivec3 hitPos = glm::ivec3{0};
    glm::ivec3 previousHitPos = glm::ivec3{0};
    float length = 0.0f;
};

RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate);

struct Raycast2DResult {
    bool success = false;
    float length = 0.0f;
    glm::vec2 hitPosition{0};
    glm::vec2 previousHitPosition{0};
};

template <class T>
Raycast2DResult raycast2D(const glm::vec2& origin, const glm::vec2& direction, float maxLength, T predicate) {
    return {};
}
}